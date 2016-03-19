$(function(){
    $('#ag').click(function(){
        if ($('#agreed').attr('checked') != 'checked')
        {

        }
        else
            $.get('/reagree', function()
            {
                location.href=location.href;
            });
    });
});