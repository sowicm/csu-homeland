$(function(){
    $('#user_pwd').holder();
    $('#user_pwdcfm').holder();
    $('#agreed').change(function(){
        $('#newpwd_submit').attr('disabled', ($(this).attr('checked') == 'checked' ? false : true));
    });
    $('#newpwd_submit').click(function(){
        if ($('#agreed').attr('checked') != 'checked')
        {
            
        }
        else if ($('#user_pwd').val().length < 8)
            alert('密码不能小于8位！');
        else if ($('#user_pwdcfm').val() != $('#user_pwd').val())
            alert('两次密码不一样!');
        else
            $.post('/home?createpass', {pwd:$('#user_pwd').val()}, function()
            {
                location.href=location.href;
            });
    });
});