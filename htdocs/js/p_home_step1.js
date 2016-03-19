$(function(){
    $('#user_email').holder();
    $('#email_submit').click(function(){
        if (!checkEmail($('#user_email').val()))
            alert('邮箱格式不合法!');
        else
            $.post('/sendverifymail', {email:$('#user_email').val()}, function()
            {
                msgBox('Information', '提交成功！请去您的邮箱检查验证邮件。');
                $('#skip').html('由于人数太多，根据目前服务器设备的限制，可能会收不到邮件，<a href="/skipverify">点此</a>跳过验证步骤');
            });
    });
});