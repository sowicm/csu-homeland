function reply_submit(e)
{
    var s = $(e);
    if (s.val().length < 1)
        msgBox('OhOh', '发表内容不能为空!');
    else
    {
        var parents = s.parentsUntil('ol');
        var li = parents[parents.length - 1];
        $.post('/home?reply', {tid:li.getAttribute('tid'),txt:s.val()}, function()
        {
            s.val('');
            s.next().children('.ninput').html('0');
            $(li).find('.replies').load('/guestbook?t=' + $(li).attr('tid') + '&p=1');
            s.focusout();
        });
    }
}
function onRepliesLoad(){
    $('.replies li').hover(function(){
        $(this).find('.op').show();
    }).mouseleave(function(){
            $(this).find('.op').hide();
        });
    $('.rebtn a').click(function(){
        $(this).parent().parent().parent().parent().parent().parent().parent().find('.reply_default').click();
        return false;
    });
};
function onSaysLoad(){
    $('.says .sl').each(function(){
        $(this).find('.replies').load('/guestbook?t=' + $(this).attr('tid') + '&p=1');
    });
    $('.reply_default').click(function(){
        $(this).hide();
        $(this).next().show();
        $(this).next().find('textarea').focus();
    });
    $('.cmtbtn a').click(function(){
        $(this).parent().parent().parent().parent().find('.reply_default').click();
        return false;
    });
    $('.reply_real textarea').focusout(function(){
        if ($(this).val() == '')
        {
            $(this).parent().parent().hide();
            $(this).parent().parent().prev().show();
        }
    }).focus(function(){
        $(this).next().children('.ninput').html($(this).val().length);
    });
    $('.reply_real').each(function(){
        var d = $(this).find('textarea')[0];
        d.oninput = function(){
            $(this).next().children('.ninput').html($(this).val().length);
        };
        d.onpropertychange = function(){
            $(this).next().children('.ninput').html($(this).val().length);
        };
        d.onkeyup = function(event){
            if (event.ctrlKey && event.keyCode == 13)
                reply_submit(this);
        }
    });
    $('.reply_real button').click(function(){ reply_submit($(this).prev().find('textarea')[0]) });
    $('.pagenumber').click(function(){ loadSays(parseInt($(this).html())) });
    $('.prevpage').click(function(){ loadSays(parseInt($(this).parent().find('.curpage').html()) - 1)});
    $('.nextpage').click(function(){ loadSays(parseInt($(this).parent().find('.curpage').html()) + 1)});
    $('.pagebar button').click(function(){ loadSays($(this).prev().prev().val())});
}
onSaysLoad();
onRepliesLoad();