$(document).ready(function () {
	console.log("Web size adjusted");
	var nav = document.getElementById("layoutSidenav_nav");
	nav.style.height = $(document).height()+"px";
});

$("#sidebarToggle").click(function() {
	console.log("toggle nav");
	var nav = document.getElementById("layoutSidenav_nav");
	if (nav.style.left == "0px") {
		nav.style.left = "-200px";
	} else {
		nav.style.left = "0px";
	}

	var main = document.getElementsByTagName("main")[0];
	main.classList.toggle("main-content-toggle");
	//main.style.width = a;
});