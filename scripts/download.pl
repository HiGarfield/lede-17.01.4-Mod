#!/usr/bin/env perl
# 
# Copyright (C) 2006 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

use strict;
use warnings;
use File::Basename;
use File::Copy;
use Text::ParseWords;

@ARGV > 2 or die "Syntax: $0 <target dir> <filename> <hash> <url filename> [<mirror> ...]\n";

my $url_filename;
my $target = glob(shift @ARGV);
my $filename = shift @ARGV;
my $file_hash = shift @ARGV;
$url_filename = shift @ARGV unless $ARGV[0] =~ /:\/\//;
my $scriptdir = dirname($0);
my @mirrors;
my $ok;

$url_filename or $url_filename = $filename;

sub localmirrors {
	my @mlist;
	open LM, "$scriptdir/localmirrors" and do {
	    while (<LM>) {
			chomp $_;
			push @mlist, $_ if $_;
		}
		close LM;
	};
	open CONFIG, "<".$ENV{'TOPDIR'}."/.config" and do {
		while (<CONFIG>) {
			/^CONFIG_LOCALMIRROR="(.+)"/ and do {
				chomp;
				my @local_mirrors = split(/;/, $1);
				push @mlist, @local_mirrors;
			};
		}
		close CONFIG;
	};

	my $mirror = $ENV{'DOWNLOAD_MIRROR'};
	$mirror and push @mlist, split(/;/, $mirror);

	return @mlist;
}

sub which($) {
	my $prog = shift;
	my $res = `command -v $prog`;
	$res or return undef;
	return $res;
}

sub hash_cmd() {
	my $len = length($file_hash);
	my $cmd;

	$len == 64 and return "mkhash sha256";
	$len == 32 and return "mkhash md5";
	return undef;
}

sub tool_present {
	my $tool_name = shift;
	my $compare_line = shift;
	my $present = 0;

	if (open TOOL, "$tool_name --version 2>/dev/null |") {
		if (defined(my $line = readline TOOL)) {
			$present = 1 if $line =~ /^$compare_line /;
		}
		close TOOL;
	}

	return $present
}

sub download_cmd {
	my $url = shift;
	my $fn = shift;
	my $additional_mirrors = join(" ", map "$_/$fn", @_);

	my @chArray = ('a'..'z', 'A'..'Z', 0..9);
	my $rfn = join '', "${fn}_", map{ $chArray[int rand @chArray] } 0..9;

	if (tool_present('aria2c', 'aria2')) {
		@mirrors=();
		return join(" ", "mkdir -p $ENV{'TMPDIR'}/aria2c",
			"&&",
			"touch $ENV{'TMPDIR'}/aria2c/${rfn}_spp",
			"&&",
			qw(aria2c --stderr -c -x2 -s10 -j10 -k1M --check-certificate=false), $url, $additional_mirrors,
			"--server-stat-of=$ENV{'TMPDIR'}/aria2c/${rfn}_spp",
			"--server-stat-if=$ENV{'TMPDIR'}/aria2c/${rfn}_spp",
			"--daemon=false --no-conf",
			"-d $ENV{'TMPDIR'}/aria2c -o $rfn",
			"&&",
			"cat $ENV{'TMPDIR'}/aria2c/$rfn",
			"&&",
			"rm -f $ENV{'TMPDIR'}/aria2c/$rfn $ENV{'TMPDIR'}/aria2c/${rfn}_spp");
	} elsif (tool_present('curl', 'curl')) {
		return (qw(curl -f --connect-timeout 20 --retry 5 --location --insecure),
			shellwords($ENV{CURL_OPTIONS} || ''),
			$url);
	} else {
		return (qw(wget --tries=5 --timeout=20 --output-document=- --no-check-certificate),
			shellwords($ENV{WGET_OPTIONS} || ''),
			$url);
	}
}

my $hash_cmd = hash_cmd();

sub download
{
	my $mirror = shift;
	my $download_filename = shift;
	my @mrs = @_;

	$mirror =~ s!/$!!;

	if ($mirror =~ s!^file://!!) {
		if (! -d "$mirror") {
			print STDERR "Wrong local cache directory -$mirror-.\n";
			cleanup();
			return;
		}

		if (! -d "$target") {
			system("mkdir", "-p", "$target/");
		}

		if (! open TMPDLS, "find $mirror -follow -name $filename 2>/dev/null |") {
			print("Failed to search for $filename in $mirror\n");
			return;
		}

		my $link;

		while (defined(my $line = readline TMPDLS)) {
			chomp ($link = $line);
			if ($. > 1) {
				print("$. or more instances of $filename in $mirror found . Only one instance allowed.\n");
				return;
			}
		}

		close TMPDLS;

		if (! $link) {
			print("No instances of $filename found in $mirror.\n");
			return;
		}

		print("Copying $filename from $link\n");
		copy($link, "$target/$filename.dl");

		$hash_cmd and do {
			if (system("cat '$target/$filename.dl' | $hash_cmd > '$target/$filename.hash'")) {
				print("Failed to generate hash for $filename\n");
				return;
			}
		};
	} else {
		my @cmd = download_cmd("$mirror/$download_filename", $download_filename, @mrs);
		print STDERR "+ ".join(" ",@cmd)."\n";
		open(FETCH_FD, '-|', @cmd) or die "Cannot launch aria2, curl or wget.\n";
		$hash_cmd and do {
			open MD5SUM, "| $hash_cmd > '$target/$filename.hash'" or die "Cannot launch $hash_cmd.\n";
		};
		open OUTPUT, "> $target/$filename.dl" or die "Cannot create file $target/$filename.dl: $!\n";
		my $buffer;
		while (read FETCH_FD, $buffer, 1048576) {
			$hash_cmd and print MD5SUM $buffer;
			print OUTPUT $buffer;
		}
		$hash_cmd and close MD5SUM;
		close FETCH_FD;
		close OUTPUT;

		if ($? >> 8) {
			print STDERR "Download failed.\n";
			cleanup();
			return;
		}
	}

	$hash_cmd and do {
		my $sum = `cat "$target/$filename.hash"`;
		$sum =~ /^(\w+)\s*/ or die "Could not generate file hash\n";
		$sum = $1;

		if ($sum ne $file_hash) {
			print STDERR "Hash of the downloaded file does not match (file: $sum, requested: $file_hash) - deleting download.\n";
			cleanup();
			return;
		}
	};

	unlink "$target/$filename";
	system("mv", "$target/$filename.dl", "$target/$filename");
	cleanup();
}

sub cleanup
{
	unlink "$target/$filename.dl";
	unlink "$target/$filename.hash";
}

@mirrors = localmirrors();

foreach my $mirror (@ARGV) {
	if ($mirror =~ /^\@SF\/(.+)$/) {
		# give sourceforge a few more tries, because it redirects to different mirrors
		for (1 .. 5) {
			push @mirrors, "https://downloads.sourceforge.net/$1";
		}
	} elsif ($mirror =~ /^\@OPENWRT$/) {
		# use OpenWrt source server directly
	} elsif ($mirror =~ /^\@DEBIAN\/(.+)$/) {
		push @mirrors, "https://ftp.debian.org/debian/$1";
		push @mirrors, "https://mirror.leaseweb.com/debian/$1";
		push @mirrors, "https://mirror.netcologne.de/debian/$1";
		push @mirrors, "https://mirrors.tuna.tsinghua.edu.cn/debian/$1";
		push @mirrors, "https://mirrors.ustc.edu.cn/debian/$1";
	} elsif ($mirror =~ /^\@APACHE\/(.+)$/) {
		push @mirrors, "https://dlcdn.apache.org/$1";
		push @mirrors, "https://mirror.aarnet.edu.au/pub/apache/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/apache/$1";
		push @mirrors, "https://archive.apache.org/dist/$1";
		push @mirrors, "https://mirror.cogentco.com/pub/apache/$1";
		push @mirrors, "https://mirror.navercorp.com/apache/$1";
		push @mirrors, "https://ftp.jaist.ac.jp/pub/apache/$1";
		push @mirrors, "https://apache.cs.utah.edu/apache.org/$1";
		push @mirrors, "http://apache.mirrors.ovh.net/ftp.apache.org/dist/$1";
		push @mirrors, "https://mirrors.tuna.tsinghua.edu.cn/apache/$1";
		push @mirrors, "https://mirrors.ustc.edu.cn/apache/$1";
	} elsif ($mirror =~ /^\@GITHUB\/(.+)$/) {
		# give github a few more tries (different mirrors)
		for (1 .. 5) {
			push @mirrors, "https://raw.githubusercontent.com/$1";
		}
	} elsif ($mirror =~ /^\@GNU\/(.+)$/) {
		push @mirrors, "https://ftpmirror.gnu.org/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/gnu/$1";
		push @mirrors, "https://mirror.netcologne.de/gnu/$1";
		push @mirrors, "https://ftp.kddilabs.jp/GNU/gnu/$1";
		push @mirrors, "https://www.nic.funet.fi/pub/gnu/gnu/$1";
		push @mirrors, "https://mirror.navercorp.com/gnu/$1";
		push @mirrors, "https://mirrors.rit.edu/gnu/$1";
		push @mirrors, "https://ftp.gnu.org/gnu/$1";
		push @mirrors, "https://mirrors.tuna.tsinghua.edu.cn/gnu/$1";
		push @mirrors, "https://mirrors.ustc.edu.cn/gnu/$1";
	} elsif ($mirror =~ /^\@SAVANNAH\/(.+)$/) {
		push @mirrors, "https://download.savannah.nongnu.org/releases/$1";
		push @mirrors, "https://mirror.netcologne.de/savannah/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/nongnu/$1";
		push @mirrors, "https://ftp.acc.umu.se/mirror/gnu.org/savannah/$1";
		push @mirrors, "https://nongnu.uib.no/$1";
		push @mirrors, "https://cdimage.debian.org/mirror/gnu.org/savannah/$1";
	} elsif ($mirror =~ /^\@KERNEL\/(.+)$/) {
		my @extra = ( $1 );
		if ($filename =~ /linux-\d+\.\d+(?:\.\d+)?-rc/) {
			push @extra, "$extra[0]/testing";
		} elsif ($filename =~ /linux-(\d+\.\d+(?:\.\d+)?)/) {
			push @extra, "$extra[0]/longterm/v$1";
		}
		foreach my $dir (@extra) {
			push @mirrors, "https://cdn.kernel.org/pub/$dir";
			push @mirrors, "https://mirrors.mit.edu/kernel/$dir";
			push @mirrors, "http://ftp.nara.wide.ad.jp/pub/kernel.org/$dir";
			push @mirrors, "http://www.ring.gr.jp/archives/linux/kernel.org/$dir";
			push @mirrors, "https://www.mirrorservice.org/sites/ftp.kernel.org/pub/$dir";
			push @mirrors, "https://mirrors.ustc.edu.cn/kernel.org/$dir";
		}
	} elsif ($mirror =~ /^\@GNOME\/(.+)$/) {
		push @mirrors, "https://download.gnome.org/sources/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/gnome/sources/$1";
		push @mirrors, "https://ftp.acc.umu.se/pub/GNOME/sources/$1";
		push @mirrors, "http://ftp.nara.wide.ad.jp/pub/X11/GNOME/sources/$1";
	} else {
		push @mirrors, $mirror;
	}
}

push @mirrors, 'https://sources.cdn.openwrt.org';
push @mirrors, 'https://sources.openwrt.org';
push @mirrors, 'https://mirror2.openwrt.org/sources';

if (-f "$target/$filename") {
	$hash_cmd and do {
		if (system("cat '$target/$filename' | $hash_cmd > '$target/$filename.hash'")) {
			die "Failed to generate hash for $filename\n";
		}

		my $sum = `cat "$target/$filename.hash"`;
		$sum =~ /^(\w+)\s*/ or die "Could not generate file hash\n";
		$sum = $1;

		cleanup();
		exit 0 if $sum eq $file_hash;

		die "Hash of the local file $filename does not match (file: $sum, requested: $file_hash) - deleting download.\n";
		unlink "$target/$filename";
	};
}

while (!-f "$target/$filename") {
	my $mirror = shift @mirrors;
	$mirror or die "No more mirrors to try - giving up.\n";

	download($mirror, $url_filename, @mirrors);
	if (!-f "$target/$filename" && $url_filename ne $filename) {
		download($mirror, $filename, @mirrors);
	}
}

$SIG{INT} = \&cleanup;
