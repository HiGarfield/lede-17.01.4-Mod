/*
 * dawn.js - periodic refresh of the DAWN status pages
 * (LEDE 17.01 backport of luci-app-dawn)
 *
 * The pages render their tables on the server side and expose a small
 * fragment endpoint (network_overview_body / hearing_map_body) which
 * returns the very same markup without header and footer. This helper
 * simply swaps the fragment into the page every few seconds.
 *
 * It relies on xhr.js which is included by every LuCI 17.01 theme.
 */

var DawnPoll = function(id, url, interval) {
	var container = document.getElementById(id);

	if (!container || typeof(XHR) == 'undefined')
		return;

	XHR.poll(interval || 5, url, null, function(xhr) {
		if (xhr && xhr.status == 200 && xhr.responseText)
			container.innerHTML = xhr.responseText;
	});
};
