$(function(){
    //$('#wrapper').css('min-height', $(window).height() + 10);
    $('.w_r').css('height', $(window).height()).css('width', $('#wrapper').innerWidth() - $('.w_l').outerWidth(true));

    $('#mlogin').click(function(){
        var pane = $('<div class="pane"></div>');
        $('body').append(pane);
        var pane_main = $('<div class="pane_main"></div>');
        pane.append(pane_main);
        pane_main.append('<ul><li><span class="pane_w">验证密码： </span><input name="pwd"/></li><li><span class="pane_w">新登录名：</span><input name="newlogin"/></li></ul>');
        var buttons = $('<div style="float:right"></div>');
        var save = $('<button class="save">保存</button>');
        var cancel = $('<button class="cancel">取消</button>');
        buttons.append(save).append(cancel);
        pane_main.append(buttons);
        cancel.click(function(){
            pane.hide();
        });
    });
});