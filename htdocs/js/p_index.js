function login()
{
    if ($('#user_id').val().length < 5)
        alert('用户名错误！');
    else if ($('#user_pwd').val().length < 8)
        alert('密码错误！');
    else
    {
        var remember = ($('#remember').attr('checked') == 'checked' ? 1 : 0);
        $.post('/login', {id:$('#user_id').val(), pwd:$('#user_pwd').val(), r:remember})
            .success(function(data){
                $('#user_pwd').val('');
                if (data == "1")
                    location.href = "home";
                else if (data == "2")
                    $.getScript('../js/fun.js');
                else
                    msgBox('登录失败', '学号或密码错误，若您确定无误，则可能被其他人抢先登录了，请从左下角反馈。');
            })
            .error(function()
            {
                alert('服务器又抽风了，请稍后重试！');
            });
    }
};
var urlPics = [
    'original_2hNR_0daa0002014a125f.jpg',
    'original_3ZUr_34f900038e461263.jpg',
    'original_4Gm7_65780001277b125f.jpg',
    'original_5NC2_33e100038e441263.jpg',
    'original_6Rty_0d6a000203a2125f.jpg',
    'original_9mWg_1d3e00038c621261.jpg',
    'original_541x_0dbe0001f555125f.jpg',
    'original_a8QR_041800038e8e1260.jpg',
    'original_BlzT_06ac0000d251125c.jpg',
    'original_BnTb_428d00038d66121a.jpg',
    'original_E5hF_068e0000d26a125c.jpg',
    'original_hcpE_06c00000d28c125c.jpg',
    'original_jEba_26120002ddad121b.jpg',
    'original_Jv6s_06480000d248125c.jpg',
    'original_lKLl_26070002dd9f121b.jpg',
    'original_Lmrt_42ce00038df2121a.jpg',
    'original_Pba4_0d420001fb31125f.jpg',
    'original_PdtQ_0d240001ffaa125f.jpg',
    'original_R68k_0d4c000203c0125f.jpg',
    'original_uqOk_656d000127a9125f.jpg',
    'original_uyt0_0d6a000203a3125f.jpg',
    'original_Vjth_06ac0000d245125c.jpg',
    'original_wbmG_57950001779a125f.jpg',
    'original_wnX1_64dc00038dd51262.jpg',
    'original_Wr6Y_3b8a00038d011263.jpg',
    'original_X9rS_0d2e000203a9125f.jpg',
    'original_YwZv_068e0000d26c125c.jpg',
    'original_ZFTI_6dc100038cd2121c.jpg',
    'original_ZsJ8_0d38000203d7125f.jpg',
    'p_large_8UdL_4d7e000000855c42.jpg',
    'p_large_LNT6_4d88000000dc5c42.jpg',
    'p_large_NLjW_12ad00003ac75c44.jpg',
    'p_large_PUrc_33b900003b615c71.jpg',
    'p_large_qgst_4a2700003b085c41.jpg',
    'p_large_rqII_7694000007911262.jpg',
    'p_large_XZ4Y_3eef00003a775c6f.jpg',
    'p_large_yqfR_238f000057131262.jpg'
];

$(function(){
    $('#pics img').attr('src', '../images/school/' + urlPics[Math.floor(Math.random() * urlPics.length)]);
    $('#wrapper').hide();
    $('#pics').hide();
    $('#cnews').hide();
    $('#login').hide();
    $('#enlist_bar').hide();
    $('#comment_bar').hide();
    $('#footer').hide();
    $('.logo').css('left', '-300px').animate({left:0},1000);
    $('#wrapper').delay(1000).fadeIn(500);
    $('#pics').delay(1500).fadeIn(500);
    $('#cnews').delay(2000).fadeIn(500);
    $('#login').delay(2500).fadeIn(500);
    $('#enlist_bar').delay(3000).css('left', '-30px').show().animate({left:0}, 500);
    $('#comment_bar').delay(3500).css('left', '-30px').show().animate({left:0}, 500);
    $('#footer').delay(4000).fadeIn(500);
    $('#user_id').holder();
    $('#user_pwd').holder();
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
    $('#btnLogin').click(function(){login()});
    document.getElementById('user_pwd').onkeydown = function(event)
    {
        if (event.keyCode == 13)
            login();
    };

});