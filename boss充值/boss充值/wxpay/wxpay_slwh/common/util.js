var util = {
	'isWeixin':function() {
		var userAgent = navigator.userAgent;
		var index = userAgent.search("MicroMessenger");
		return index !== -1;
	},
	'jumpWithSearch':function(url, params) {
		var query = window.location.search;
		if (query) {
			query = query + (params == null ? "" : "&" + params);
		} else {
			query = (params == null) ? "" : "?" + params;
		}
		window.location = url + query;
	}
};