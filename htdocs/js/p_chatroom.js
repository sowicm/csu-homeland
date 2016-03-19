function send(){
    $.post(location.href, {txt:$('#txt').val()});
    $('#txt').val('');
}
function callback(options)
{
    switch(options.a)
    {
    case 'msg':
        $('#logol').append('<li class="logitem"><strong class="' + (options.male === 1 ? 'male' : 'female') + '">' + options.name + ':</strong>' + options.content + '</li>');
    }
}
function connect_htmlfile()
{
    transferDoc = new ActiveXObject('htmlfile');
    transferDoc.open();
    transferDoc.write(
        '<html><script type="text/javascript">document.domain=\''+document.domain+'\';</script></html>'
    );
    transferDoc.close();
    var ifr = transferDoc.createElement('iframe');
    ifr.setAttribute('src', "/chatroom/comet?r='+location.href.substring(location.href.indexOf('?r=')+3)+'");
    ifr.parent = window;
    transferDoc.body.appendChild(ifr);
    //ifr.parent.callback = callback;
    //ifr.parentWindow.callback = callback;
    ifr.parent = window;
    //alert(ifr.parentWindow);
    //transferDoc.callback = callback;
    //transferDoc.parentWindow.callback = callback;
    //alert(ifr.parent.document.body.innerHTML);
    //ifr.parent.callback({a:'msg',msg:'ohyeah'});
    alert('connected');
}
function close_htmlfile()
{
    transferDoc = null;
    CollectGarbage();
}
function read(){
    $.ajax({
        type: 'get',
        url: '/chatroom/comet?r='+location.href.substring(location.href.indexOf('?r=')+3),
        timeout: 80000
    }).success(function(data){
        $('#logol').append(data);
        read();
    }).error(function(XMLHttpRequest,textStatus,errorThrow){
        if (textStatus === 'timeout'){
            read();
        }
    });
}
function leave()
{
    $.post(location.href, {leave:'1'}).success(function(){
       location.href = 'http://' + location.host;
    });
};

$(function(){
    window.onbeforeunload = function()
    {
        return "你真的要退出聊天吗？";
    };
    $('#btnSend').click(function(){send()});
    read();
    window.onkeyup = function(event)
    {
        if (event.keyCode == 27)
            leave();
    };
    $('#btnDiscon').click(function(){leave()});
    $('#txt')[0].onkeyup = function(event)
    {
        if (event.keyCode == 13)
            send();
    }
    //if ($.browser.msie)
    //{
    //    alert('ie');
    //    connect_htmlfile();
    //}
    //else
    //{
    //    $('body').append('<iframe src="/chatroom/comet?r='+location.href.substring(location.href.indexOf('?r=')+3)+'" width="0" height="0"></iframe>');
    //}
});
