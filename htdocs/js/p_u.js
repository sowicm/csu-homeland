$(function(){
    $('#rating a').each(function(){
        $(this).click(function(){
            var score = parseInt($(this).attr('title'));
            sliderConfirm({title:'你确定要给'+$('h3').html()+'评<span style="color:red">'+score+'</span>分吗？',
                action:function(){
                    var s = 0;
                    $.ajax({
                        url: '/rating',
                        async: false,
                        type: 'post',
                        data:{did:$('#did').val(),score:score}
                    }).success(function(data){
                        s = parseInt(data);
                    });
                    return s;
                },
                callback:function(tip, ret){
                    if (ret == 1)
                    {
                        tip.css('color', '#007b09');
                        tip.html('投票成功');
                    }
                    else
                    {
                        tip.css('color', 'red');
                        if (ret == 2)
                        {
                            tip.html('一天只能对ta评分一次哦!');
                        }
                        else
                        {
                            tip.html('投票失败');
                        }
                    }
                }
            });
       });
    });
});