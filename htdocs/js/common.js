var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-31988700-1']);
_gaq.push(['_trackPageview']);

(function() {
    var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
    ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
    var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
    })();

$.fn.holder = function()
{
    var t = $(this);
    var h = t.prev();
    var ox = -1 * t.position().left;
    if (ox == 0)
        ox = parseInt(t.css('margin-left'));
    var oy = -1 * t.position().top;
    if (oy == 0)
        oy = parseInt(t.css('margin-top'));
    h.css('left', parseInt(t.css('padding-left')) + ox);
    h.css('top', parseInt(t.css('padding-top')) + oy);
    h.css('font-size', t.css('font-size'));
    h.css('color', t.css('color'));
    h.css('font-family', t.css('font-family'));
    h.css('width', t.css('width'));
    if (t.val() == '')
        h.show();
    else
        h.hide();
    t.focusin(function(){
        h.hide();
    });
    t.focusout(function(){
        if (t.val() == '')
            h.show();
    });
    h.click(function(){t.focus()});
};
/*
*  placeholder
*  author: sowicm.com
*  usage: $('#pwd').placeholder('密码');
$.fn.placeholder = function(a)
{
    if ($(this).attr('type') == 'password')
    {
        $(this)[0].setAttribute('type','text');
        $(this).val(a);
        $(this).focusin(function(){
            if ($(this).val() == a)
                $(this).val('');
        });
        $(this).focusout(function(){
            if ($(this).val() == '')
            {
                $(this)[0].setAttribute('type','text');
                $(this).val(a);
            }
        });
        $(this).change(function(){
            $(this)[0].setAttribute('type','password');
        }).keydown(function(){
                $(this)[0].setAttribute('type','password');
        });
    }
    else
    {
        $(this).val(a);
        $(this).focusin(function(){
            if ($(this).val() == a)
                $(this).val('');
        });
        $(this).focusout(function(){
            if ($(this).val() == '')
                $(this).val(a);
        });
    }
};
*/
function createWnd(width, height, title, html)
{
    var wnd = $('<div class="wnd" style="width:'+width+'px;height:'+height+'px;"></div>').hide();
    $('body').append(wnd);
    var isMouseDown = false;
    var offsetX;
    var offsetY;
    var titlebar = $('<div class="wnd_title">'+title+'</div>')
        .mousedown(function(e){
            offsetX = e.clientX - parseInt(wnd.css('left'));
            offsetY = e.clientY - parseInt(wnd.css('top'));
            isMouseDown = true;
        })
        .mousemove(function(e){
            if (isMouseDown)
            {
                wnd.css('left', e.clientX - offsetX).css('top', e.clientY - offsetY);
            }
        })
        .mouseup(function(){
            isMouseDown = false;
        })
        .mouseout(function(){
            isMouseDown = false;
        });
    var closebtn = $('<span class="closebtn" title="关闭">×</span>').hover(function(){
        $(this).addClass('closebtnhover');
    }, function(){
        $(this).removeClass('closebtnhover');
    }).click(function(){
        wnd.hide();
    }).css('right', '0');
    var client = $('<div class="client" style="position:relative;overflow:auto;width:100%;height:'+(height-31)+'px;"></div>')
        .append(html);
    var wnd_all = $('<div class="wnd_all"></div>');
    wnd_all.append( titlebar.append(closebtn) ).append(client);
    //wnd.append(closebtn).append(titlebar).append(client);
    wnd.append(wnd_all);
    wnd.css('top', ($(window).height() - parseInt(wnd.css('height'))) / 2);
    wnd.css('left', ($(window).width() - parseInt(wnd.css('width'))) / 2);
    wnd.show();
};
function sliderConfirm(options)
{
    var wrapper = $('<div style="margin:30px"></div>');
    var title = $('<div style="margin:10px 0">' + options.title + '</div>');
    var slider = $('<div class="slider"></div>');
    var bar = $('<div class="bar"></div>');
    var thumb = $('<div class="thumb"></div>');
    var icon = $('<div class="icon"></div>');
    var tip = $('<div class="tip">滑动确认此操作</div>');

    thumb.draggable({
        revert: function(){
            return (parseInt(thumb.css('left')) <= 150);
        },
        containment: bar,
        axis: 'x',
        stop: function(event, ui) {
            if (ui.position.left > 150) {
                icon.css('background-position', '-16px 0');
                var ret = options.action();
                options.callback(tip, ret);
            }
        }
    });
    wrapper.append(title).append(slider);
    slider.append(bar).append(icon).append(tip);
    bar.append(thumb);
    createWnd(400, 200, '请确认操作', wrapper);
}
function msgBox(title, msg)
{
    var html = '<div style="text-align:center;margin:0 auto;padding:5px 10px;width:300px;">'+msg+'</div>';
    createWnd(400, 120, title, html);
};
function checkEmail(str)
{
    return /^[\w]+[\w\.]*@[\w-]+\.[\w\.]+$/.test(str);
};
function checkStuId(str)
{
    if (str.length < 6)
        return false;
    return /^[a-z,A-Z,0-9]+$/.test(str);
};
function checkPhone(str)
{
    return /^\d{11}$/.test(str);
};
$(function(){
    $('input,textarea').focusin(function(){
        $(this).addClass('current');
    }).focusout(function(){
        $(this).removeClass('current');
    });
    $('.closebtn').hover(function(){
        $(this).addClass('closebtnhover');
    }, function(){
        $(this).removeClass('closebtnhover');
    });
    $('.closebtn').click(function(){
        $(this).parent().hide();
    });
    $('#enlist_email').holder();
    $('#enlist_id').holder();
    $('#enlist_phone').holder();
    $('#enlist_txt').holder();
    $('#comment_email').holder();
    $('#comment_txt').holder();
    /* click -> hover */
    $('#enlist_bar').hover(function(){
        $('#enlist').css('left', '-330px').show().animate({left:0}, 250);
    });
    $('#comment_bar').hover(function(){
        $('#comment').css('left', '-330px').show().animate({left:0}, 250);
    });
    var d = document.getElementById('enlist_phone');
    d.setAttribute('maxlength',11);
    d.onkeydown = function(event){
        return (/[\d\t\b]/.test(String.fromCharCode(event.keyCode)));
    };
    $('input[name="email"]').each(function(){
        var e = $(this)[0];
        e.oninput = function()
        {
            $('input[name="email"]').each(function(){
                if ($(this)[0] != e)
                {
                    $(this).val(e.value);
                    $(this).holder();
                }
            });
        };
        e.onblur = function(){
            var v = e.value;
            if (/^\d+$/.test(v))
                $('input[name="email"]').each(function(){
                    $(this).val(v + '@qq.com');
                    $(this).holder();
                });
        };
    });
    $('#enlist_submit').click(function(){
        if (!checkEmail($('#enlist_email').val()))
        {
            alert('邮箱格式不合法!');
        }
        else if (!checkStuId($('#enlist_id').val()))
        {
            alert('Are you kidding me! 学号写错了吧...');
        }
        else if (!checkPhone($('#enlist_phone').val()))
        {
            alert('手机号不合法！');
        }
        else if ($('#enlist_txt').val() == '')
        {
            alert('请说点什么吧！');
        }
        else
        {
            $.post('/enlist_submit', {email:$('#enlist_email').val(),stuid:$('#enlist_id').val(),phone:$('#enlist_phone').val(),txt:$('#enlist_txt').val()}, function()
            {
                $('#enlist_txt').val('');
                msgBox('Information', '提交成功！')
            });
        }
    });
    $('#comment_submit').click(function(){
        if (!checkEmail($('#comment_email').val()))
        {
            alert('邮箱格式不合法!');
        }
        else if ($('#comment_txt').val() == '')
        {
            alert('请说点什么吧！');
        }
        else
        {
            $.post('/suggestion_submit', {email:$('#comment_email').val(),txt:$('#comment_txt').val()}, function()
            {
                $('#comment_txt').val('');
                msgBox('Information', '提交成功！')
            });
        }
    });
    $('#account').hover(function(){
        $('#acp').show();
    }, function(){
        $('#acp').hide();
    });
});