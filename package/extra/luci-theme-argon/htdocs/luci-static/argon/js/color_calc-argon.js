/*
 *  The background color of the [Light Mode] subtabs follow the custom primary color and reduce its transparency
 *  Author: SpeedPartner
 */

/*
 *  Get hex for the [Light mode] Primary Color ,then reduce it to 25% transparency and convert it to RGBA value
 */
const normalizeHexColor_primary_light = (hex) => {
    if (typeof hex !== "string") {
        return null;
    }
    const value = hex.trim();
    if (/^#[0-9a-fA-F]{3}$/.test(value)) {
        return "#" + value[1] + value[1] + value[2] + value[2] + value[3] + value[3];
    }
    return /^#[0-9a-fA-F]{6}$/.test(value) ? value : null;
};
const defaultHexColor_primary_light = "#5e72e4";
const hexColor_primary_light = normalizeHexColor_primary_light(
    getComputedStyle(document.documentElement).getPropertyValue('--primary')
);
const hexToRgba_primary_light = (hex) => {
    const safeHex = hex || defaultHexColor_primary_light;
    const r = parseInt(safeHex.substring(1, 3), 16);
    const g = parseInt(safeHex.substring(3, 5), 16);
    const b = parseInt(safeHex.substring(5, 7), 16);
    const a = 0.15;
    return [r, g, b, a];
};
const rgbaColor_primary_light = hexToRgba_primary_light(hexColor_primary_light);

/*
 *  Constitute a css color variable named light-subtabs-background
 */
document.documentElement.style.setProperty('--light-subtabs-background', `rgba(${rgbaColor_primary_light.join(",")})`);
