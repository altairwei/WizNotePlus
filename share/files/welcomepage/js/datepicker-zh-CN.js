/* Chinese initialisation for the jQuery UI date picker plugin. */
/* Written by Cloudream (cloudream@gmail.com). */
( function( factory ) {
	if ( typeof define === "function" && define.amd ) {

		// AMD. Register as an anonymous module.
		define( [ "../widgets/datepicker" ], factory );
	} else {

		// Browser globals
		factory( jQuery.datepicker );
	}
}( function( datepicker ) {

datepicker.regional[ "zh-CN" ] = {
	closeText: "??",
	prevText: "&#x3C;??",
	nextText: "??&#x3E;",
	currentText: "??",
	monthNames: [ "??","??","??","??","??","??",
	"??","??","??","??","???","???" ],
	monthNamesShort: [ "??","??","??","??","??","??",
	"??","??","??","??","???","???" ],
	dayNames: [ "???","???","???","???","???","???","???" ],
	dayNamesShort: [ "??","??","??","??","??","??","??" ],
	dayNamesMin: [ "?","?","?","?","?","?","?" ],
	weekHeader: "?",
	dateFormat: "yy-mm-dd",
	firstDay: 1,
	isRTL: false,
	showMonthAfterYear: true,
	yearSuffix: "?" };
datepicker.setDefaults( datepicker.regional[ "zh-CN" ] );

return datepicker.regional[ "zh-CN" ];

} ) );