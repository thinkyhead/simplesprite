(function(){
  var key='simplesprite-docs-theme';
  var theme=getCookie(key)||'dark';
  apply(theme);
})();

function getCookie(name){
  var m=document.cookie.match(new RegExp("(?:^|; )"+name+"=([^;]+)"));
  return m?decodeURIComponent(m[1]):null;
}
function setCookie(name,value,days){
  var d=new Date();
  d.setTime(d.getTime()+(days*24*60*60*1000));
  document.cookie=name+"="+encodeURIComponent(value)+";expires="+d.toUTCString()+";path=/";
}
function apply(t){document.documentElement.setAttribute('data-theme',t);setCookie(key,t,365);}
function toggleTheme(){
  var cur=document.documentElement.getAttribute('data-theme');
  apply(cur==='dark'?'light':'dark');
}
