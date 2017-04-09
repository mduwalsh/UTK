"use strict";StackExchange.mockups=function(){function t(t,e,i,n,s){function o(t,e,i){for(var n=-1,s=-1;;){if(s=e.indexOf(t,s+1),-1==s)break;(0>n||Math.abs(s-i)<Math.abs(s-n))&&(n=s)}return n}return t.replace(new RegExp("<!-- Begin mockup[^>]*? -->\\s*!\\[[^\\]]*\\]\\((http://[^ )]+)[^)]*\\)\\s*<!-- End mockup -->","g"),function(t,a,r){var l={"payload":a.replace(/[^-A-Za-z0-9+&@#\/%?=~_|!:,.;\(\)]/g,""),"pos":o(t,e,r),"len":t.length};return-1===l.pos?t:(s.push(l),t+"\n\n"+i+n+"-"+(s.length-1)+"%")})}function e(){StackExchange.externalEditor.init({"thingName":"mockup","thingFinder":t,"getIframeUrl":function(t){var e="/plugins/mockups/editor";return t&&(e+="?edit="+encodeURIComponent(t)),e},"buttonTooltip":"UI wireframe","buttonImageUrl":"/content/balsamiq/wmd-mockup-button.png?v=4","onShow":function(t){window.addMockupToEditor=t},"onRemove":function(){window.addMockupToEditor=null;try{delete window.addMockupToEditor}catch(t){}}})}return{"init":e}}(),StackExchange.schematics=function(){function t(){if(!window.postMessage)return n;var t=document.createElement("div");t.innerHTML="<svg/>";var e="http://www.w3.org/2000/svg"==(t.firstChild&&t.firstChild.namespaceURI);if(!e)return n;var i=navigator.userAgent;return/Firefox|Chrome/.test(i)?a:/Apple/.test(navigator.vendor)||/Opera/.test(i)?o:s}function e(t,e,i,n,s){function o(t,e,i){for(var n=-1,s=-1;;){if(s=e.indexOf(t,s+1),-1==s)break;(0>n||Math.abs(s-i)<Math.abs(s-n))&&(n=s)}return n}return t.replace(new RegExp("<!-- Begin schematic[^>]*? -->\\s*!\\[[^\\]]*\\]\\((http://[^ )]+)[^)]*\\)\\s*<!-- End schematic -->","g"),function(t,a,r){var l={"payload":a.replace(/[^-A-Za-z0-9+&@#\/%?=~_|!:,.;\(\)]/g,""),"pos":o(t,e,r),"len":t.length};return-1===l.pos?t:(s.push(l),t+"\n\n"+i+n+"-"+(s.length-1)+"%")})}function i(){var i;StackExchange.externalEditor.init({"thingName":"schematic","thingFinder":e,"getIframeUrl":function(t){var e="/plugins/schematics/editor";return t&&(e+="?edit="+encodeURIComponent(t)),e},"buttonTooltip":"Schematic","buttonImageUrl":"/content/electronics/img/wmd-schematic-button.png?v=1","checkSupport":function(){var e=t();switch(e){case a:return!0;case o:return confirm("Your browser is not officially supported by the schematics editor; however it has been reported to work. Launch the editor?");case s:return confirm("Your browser is not officially supported by the schematics editor; it may or may not work. Launch the editor anyway?");case n:return alert("Sorry, your browser does not support all the necessary features for the schematics editor."),!1}},"onShow":function(t){var e=$("<div class='popup' />").css("z-index",1111).text("Loading editor").appendTo("body").show().addSpinner({"marginLeft":5}).center({"dy":-200});$("<div style='text-align:right;margin-top: 10px' />").append($("<button>cancel</button>").click(function(){e.remove(),t()})).appendTo(e),i=function(i){if(i=i.originalEvent,"https://www.circuitlab.com"===i.origin){i.data||t();var n=$.parseJSON(i.data);if(n&&"success"===n.load)return e.remove(),void 0;if(n&&n.edit_url&&n.image_url){n.fkey=StackExchange.options.user.fkey;var s=$("<div class='popup' />").css("z-index",1111).appendTo("body").show(),o=function(){s.text("Storing image").addSpinner({"marginLeft":5}).center(),$.post("/plugins/schematics/save",n).done(function(e){s.remove(),t(e.img)}).fail(function(t){if(409===t.status){var e="Storing aborted";t.responseText.length<200&&(e=t.responseText),s.text(e+", will retry shortly").addSpinner({"marginLeft":5}).center(),setTimeout(o,1e4)}else s.remove(),alert("Failed to upload the schematic image.")})};o()}}},$(window).on("message",i)},"onRemove":function(){$(window).off("message",i)}})}var n=0,s=1,o=2,a=3;return{"init":i}}(),StackExchange.externalEditor=function(){function t(t){function e(t,e){function d(e){function n(){b.add("#lightbox").remove(),h()}var s=m||v.caret(),r=v[0].value||"",l=e?e.pos:s.start,u=e?e.len:s.end-s.start,d=r.substring(0,l),p=r.substring(l+u);m=null;var g=function(e,o){if(!e)return setTimeout(n,0),v.focus(),void 0;StackExchange.navPrevention.start();var a=void 0===o?i(e):o,r=d.replace(/(?:\r\n|\r|\n){1,2}$/,""),l=r+a+p.replace(/^(?:\r\n|\r|\n){1,2}/,""),c=s.start+a.length-d.length+r.length;setTimeout(function(){t.textOperation(function(){v.val(l).focus().caret(c,c)}),n()},0)},f=$('<div id="lightbox" class="esc-remove" />');f.appendTo($("body")).css({"height":$(document).height(),"opacity":.5}).fadeIn("fast");var b=null;if(o){var x=o(e?e.payload:null);b=$('<iframe class="esc-remove" src="'+x+'" width="95%" height="95%" style="position: fixed; top: 2.5%; left: 2.5%; background: white; z-index: 1001" />')}else{var w=a(e?e.payload:null);b=$('<div class="esc-remove" style="position: fixed; top: 2.5%; left: 2.5%; width: 95%; height: 95%; background: white; z-index: 1001"></div>'),b.append(w)}b.appendTo($("body")),$(window).resize(),c(g)}$('<style type="text/css"> .wmd-'+n+"-button span { background-position: 0 0; } .wmd-"+n+"-button:hover span { background-position: 0 -40px; }</style>)").appendTo("head");var p,g,m,f=t.getConverter().hooks,v=$("#wmd-input"+e);f.chain("preConversion",function(t){var e=(t.match(/%/g)||[]).length,i=v[0].value||"";return p=new Array(e+2).join("%"),g=[],s(t,i,p,n,g)}),f.chain("postConversion",function(t){return t.replace(new RegExp(p+n+"-(\\d+)%","g"),function(t,e){return"<sup><a href='#' class='edit-"+n+"' data-id='"+e+"'>edit the above "+n+"</a></sup>"})});var b="The "+n+" editor does not support touch devices.",x=!1;$("#wmd-preview"+e).on("touchend",function(){x=!0}).on("click","a.edit-"+n,function(){return x?(alert(b),x=!1,!1):(x=!1,(!u||u())&&d(g[$(this).attr("data-id")]),!1)}),$("#wmd-input"+e).keyup(function(t){t.shiftKey||t.altKey||t.metaKey||!t.ctrlKey||77!==t.which||(!u||u())&&d()}),setTimeout(function(){var t=($("#wmd-button-bar"+e),$("#wmd-image-button"+e)),i=parseInt(t.css("left"));t.nextAll("li").each(function(){var t=$(this),e=parseInt(t.css("left"));t.css("left",e+25)});var s=$("<li class='wmd-button wmd-"+n+"-button' style='left:"+(i+25)+"px' id='wmd-"+n+"-button"+e+"' title='"+r+" Ctrl-M' />").insertAfter(t),o=!1,a=$("<span />").css({"backgroundImage":"url("+l+")"}).appendTo(s).on("touchend",function(){o=!0}).click(function(){return o?(alert(b),o=!1,void 0):(o=!1,(!u||u())&&d(),void 0)});$.browser.msie&&a.mousedown(function(){m=v.caret()})},0)}function i(t){return('\n\n<!-- Begin {THING}: In order to preserve an editable {THING}, please\n     don\'t edit this section directly.\n     Click the "edit" link below the image in the preview instead. -->\n\n![{THING}]('+t+")\n\n<!-- End {THING} -->\n\n").replace(/{THING}/g,n)}var n=t.thingName,s=t.thingFinder,o=t.getIframeUrl,a=t.getDivContent,r=t.buttonTooltip,l=t.buttonImageUrl,c=t.onShow,h=t.onRemove,u=t.checkSupport;StackExchange.MarkdownEditor.creationCallbacks.add(e)}return{"init":t}}();