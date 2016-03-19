function s_submit()
{
    location.href='/search?n=' + $('#s').val();
};
$(function(){
    $('#s').holder();
    $('#s')[0].onkeyup = function(event){
        if (event.keyCode == 13)
             s_submit();
    };
    $('#search_submit')[0].onclick = function(){ s_submit(); };
});