# Test formula

Next should be changed

---

$$

---

<div>

<script type="text/javascript">

(function on_load() {

var elems = document.getElementsByTagName("*");
var no_of_elems = elems.length;
for (var i=0; i < no_of_elems; i++) {
    
    var childs = elems[i].childNodes;
    var no_of_child = childs.length;
    for (var j=0; j < no_of_child; j++) {
        if (childs[j].nodeType == Node.TEXT_NODE) {
            var text = childs[j].nodeValue;
            if ( text.indexOf('$$') >= 0 )
                childs[j].nodeValue = "changed";    
        }
    }
}

})();

</script>

</div>

