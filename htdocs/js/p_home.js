function say_submit()
{
    var s = $('.say_txt textarea');
    if (s.val().length < 1)
        msgBox('OhOh', '发表内容不能为空!');
    else
        $.post('/home?say', {txt:s.val()}, function()
        {
            s.val('');
            s.next().children('.ninput').html('0');
            $('#contents').load("/guestbook?p=1");
        });
}
function loadSays(p)
{
    $('#contents').load('/guestbook?p=' + p);
}
$(window).resize(function(){
    $('.say_txt textarea').css('width', '100%');
    $('.w_m').css('width', $('#wrapper').innerWidth() - $('.w_l').outerWidth(true) - $('.w_r').outerWidth(true) - 40);
    $('.say_txt textarea').css('width', '-=20px');
});
$(function(){
    $('#wrapper').css('min-height', $(window).height() + 10);
    $('.w_m').css('width', $('#wrapper').innerWidth() - $('.w_l').outerWidth(true) - $('.w_r').outerWidth(true) - 40);
    $('.say_txt textarea').css('width', '-=20px');
    $('.say_txt textarea').holder();
    $('#say_submit').click(function(){ say_submit() });
    var s = $('.say_txt textarea');
    var d = s[0];
    d.oninput = function(){
        $(this).next().children('.ninput').html($(this).val().length);
    };
    d.onpropertychange = function(){
        $(this).next().children('.ninput').html($(this).val().length);
    };
    d.onkeyup = function(event){
        if (event.ctrlKey && event.keyCode == 13)
            say_submit();
    };
    s.next().children('.ninput').html(s.val().length);
    loadSays(1);
    $('#cnews li').hover(function() {
        $(this).children('a').css('color', '#000');
        $(this).animate({ paddingLeft: '10px' }, 250);
    }, function() {
        $(this).children('a').css('color', '#3a3a3a');
        $(this).animate({ paddingLeft: '0px' }, 250);
    });
    $('#cnews li a').click(function(){
        if ($(this).attr('href') != '#')
        {
            $.get($(this).attr('href')).success(function(data){
                createWnd(800, 500, 'Notice', data);
            });
        }
        return false;
    });
    $('#ask').click(function(){
        $.get('/askfor?Conan').success(function(data){
            if (data === '1')
            {
                createWnd(200, 80, '申请成功!', '<div style="margin:10px">申请成功！<a href="/down/Conan_Tool.exe" target="_blank">点此下载</a></div>')
            }
            else if (data === '2')
                alert('名额已满了哦！');
            else
                alert('申请失败！');
        });
    });
});