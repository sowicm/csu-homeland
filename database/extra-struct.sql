-- ----------------------------
-- View structure for `tmp_enlist`
-- ----------------------------
DROP VIEW IF EXISTS `tmp_enlist`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `tmp_enlist` AS select `e`.`email` AS `email`,`u`.`display_name` AS `display_name`,`e`.`phone` AS `phone`,`e`.`txt` AS `txt`,`e`.`time_prc` AS `time_prc`,`e`.`ip` AS `ip`,`e`.`agent` AS `agent` from (`enlist` `e` join `users_stu` `u` on((`u`.`stuid` = `e`.`stuid`))) ;

-- ----------------------------
-- View structure for `v_replies`
-- ----------------------------
DROP VIEW IF EXISTS `v_replies`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_replies` AS select `r`.`says_id` AS `says_id`,`u`.`did` AS `did`,`u`.`display_name` AS `display_name`,`r`.`content` AS `content`,`r`.`time_prc` AS `time_prc` from (`says_replies` `r` join `users_stu` `u` on((`r`.`uid` = `u`.`id`))) ;

-- ----------------------------
-- View structure for `v_says`
-- ----------------------------
DROP VIEW IF EXISTS `v_says`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `v_says` AS select `says`.`says_id` AS `says_id`,`users_stu`.`did` AS `did`,`users_stu`.`display_name` AS `display_name`,`says`.`content` AS `content`,`says`.`time_prc` AS `time_prc`,`says`.`replies_count` AS `replies_count` from (`says` join `users_stu` on((`says`.`uid` = `users_stu`.`id`))) ;

-- ----------------------------
-- Procedure structure for `changeEmail`
-- ----------------------------
DROP PROCEDURE IF EXISTS `changeEmail`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `changeEmail`(in arg_uid bigint, in arg_email varchar(320), in arg_code char(32), in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    call stu_modify_info(arg_uid, 'email', arg_email, arg_ip, arg_agent);
    update `users_stu` set `email_validated`=0 where `id`=arg_uid;
    replace into `email_verify_codes` (`uid`,`code`) values (arg_uid,arg_code);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `createpass`
-- ----------------------------
DROP PROCEDURE IF EXISTS `createpass`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `createpass`(in arg_uid bigint, in arg_pwd varchar(16), in arg_pwdmd5 char(32), in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set @step = (select `step` from `users_stu` where `id`=arg_uid);
    if @step = 2 then
        call stu_modify_info(arg_uid, 'pwd', arg_pwd, arg_ip, arg_agent);
        call stu_modify_info(arg_uid, 'pwd', arg_pwdmd5, arg_ip, arg_agent);
        update `users_stu` set `step`=3,`agreed`=1 where `id`=arg_uid;
    end if;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `enlist_submit`
-- ----------------------------
DROP PROCEDURE IF EXISTS `enlist_submit`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `enlist_submit`(in arg_email varchar(320), in arg_stuid varchar(32), in arg_phone char(11), in arg_txt longtext, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    insert into enlist (email,stuid,phone,txt,time_prc,ip,agent) values(arg_email, arg_stuid, arg_phone, arg_txt,NOW(),arg_ip,arg_agent);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `rating`
-- ----------------------------
DROP PROCEDURE IF EXISTS `rating`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `rating`(in arg_uid bigint, in arg_stuid varchar(32), in arg_score tinyint, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    insert into ratings (uid, stuid, score, time_prc, ip, agent) values (arg_uid, arg_stuid, arg_score, NOW(), arg_ip, arg_agent);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `search`
-- ----------------------------
DROP PROCEDURE IF EXISTS `search`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `search`(in arg_name varchar(255), in arg_uid bigint, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    set NAMES utf8;
    insert into log_search_stu (uid,search,time_prc,ip,agent) values (arg_uid,arg_name,NOW(),arg_ip,arg_agent);
    select u.did,u.name,c.classname from si_stu u inner join si_class c on c.classid=u.class where position(arg_name in u.name) <> 0 limit 10;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `session_destroy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `session_destroy`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `session_destroy`(in arg_sid varchar(64))
BEGIN
    set time_zone = '+8:00';
    delete from `sessions` where `sid`=arg_sid;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `session_save`
-- ----------------------------
DROP PROCEDURE IF EXISTS `session_save`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `session_save`(in arg_sid varchar(64), in arg_ip varchar(40), in arg_uid bigint(20))
BEGIN
    set time_zone = '+8:00';
    insert into `sessions` (`sid`,`ip`,`uid`,`lasttime`) values (arg_sid,arg_ip,arg_uid,UNIX_TIMESTAMP());
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `session_start`
-- ----------------------------
DROP PROCEDURE IF EXISTS `session_start`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `session_start`(in arg_sid varchar(64), in arg_ip varchar(40))
BEGIN
    set time_zone = '+8:00';
    select `uid` from `sessions` where `sid`=arg_sid and `ip`=arg_ip;
    update `sessions` set `lasttime`=UNIX_TIMESTAMP() where `sid`=arg_sid;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `stu_modify_info`
-- ----------------------------
DROP PROCEDURE IF EXISTS `stu_modify_info`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `stu_modify_info`(in arg_uid bigint(20), in arg_op varchar(64), in arg_val longtext, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    set NAMES utf8;
    set @pre = concat("update `users_stu` set `", arg_op, "`=? where `id`=?");
    prepare stmt from @pre;
    set @uid = arg_uid;
    set @val = arg_val;
    execute stmt using @val, @uid;
    
    insert into `log_modified_stu` (`uid`,`option`,`value`,`time_prc`,`ip`,`agent`) values (arg_uid, arg_op, arg_val, NOW(), arg_ip, arg_agent);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `suggestion_submit`
-- ----------------------------
DROP PROCEDURE IF EXISTS `suggestion_submit`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `suggestion_submit`(in arg_email varchar(320), in arg_txt longtext, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    insert into suggestions (email,txt,time_prc,ip,agent) values(arg_email,arg_txt,NOW(),arg_ip,arg_agent);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `t_setdid`
-- ----------------------------
DROP PROCEDURE IF EXISTS `t_setdid`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `t_setdid`()
BEGIN
/*
    declare n int;
    set n = (select count(*) from si_stu where did is null);
    l : loop
        if n < 1 then 
            leave l;
        end if;
        set @did = (select s from randstring_16 limit 1);
        delete from randstring_16 where s=@did;
        update si_stu set did=@did where did is null limit 1;
        set n=n-1;
        iterate l;
    end loop l;
    */
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `user_login`
-- ----------------------------
DROP PROCEDURE IF EXISTS `user_login`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `user_login`(in arg_login varchar(16), in arg_pwd varchar(16), in arg_pwd_md5 char(32), in arg_ip varchar(40),  in arg_agent varchar(255))
BEGIN
    declare success tinyint;
    set time_zone = '+8:00';
    set success = 0;
    set @uid = (select `id` from users_stu where `login`=arg_login and `pwd`=arg_pwd_md5);
    if @uid is not null then
        set success = 1;
    else
        set @uid=(select `id` from `users_stu` where `login`=arg_login and `pwd`='');
        if @uid is not null then
            set @birthday=(select `birthday` from `si_stu` where `stuid`=arg_login);
            if @birthday is not null then
                set @birthday = (select date_format(@birthday, '%Y%m%d'));
                if @birthday = arg_pwd then
                    set success = 1;
                end if;
            end if;
        else
            set @n = (select count(*) from `users_stu` where `stuid`=arg_login);
            if @n < 1 then
                set @birthday=(select `birthday` from `si_stu` where `stuid`=arg_login);
                if @birthday is not null then
                    set @birthday = (select date_format(@birthday, '%Y%m%d'));
                    if @birthday = arg_pwd then
                        set @name = (select `name` from `si_stu` where `stuid`=arg_login);
                        set @did = (select `did` from `si_stu` where `stuid`=arg_login);
                        insert into `users_stu` (`did`,`stuid`, `login`, `display_name`, `pwd`, `registered_prc`) values (@did,arg_login, arg_login, @name, arg_pwd_md5, NOW());
                        set @uid = (select `id` from `users_stu` where `stuid`=arg_login);
                        set @n = (select count(@uid));
                        if @n > 0 then
                            set success = 1;
                        end if;
                    end if;
                end if;
            end if;
        end if;
    end if;
    if success > 0 then
        insert into `log_login_stu` (`uid`,`time_prc`,`ip`,`agent`) values (@uid,NOW(),arg_ip,arg_agent);
        set @n = (select count(*) from `sessions` where `uid`=@uid);
        update `users_stu` set `loggedin`=@n+1 where `id`=@uid;
        select @uid;
    else
        insert into `log_wlogin_stu` (`login`,`pwd`,`time_prc`,`ip`,`agent`) values (arg_login,arg_pwd,NOW(),arg_ip,arg_agent);
    end if;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `user_reply`
-- ----------------------------
DROP PROCEDURE IF EXISTS `user_reply`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `user_reply`(in arg_tid bigint, in arg_uid bigint, in arg_txt longtext, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    set NAMES utf8;
    insert into `says_replies` (`says_id`, `uid`,`content`,`time_prc`,`ip`,`agent`) values (arg_tid,arg_uid,arg_txt,NOW(),arg_ip,arg_agent);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `user_say`
-- ----------------------------
DROP PROCEDURE IF EXISTS `user_say`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `user_say`(in arg_uid bigint, in arg_txt longtext, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    set NAMES utf8;
    insert into `says` (`uid`,`content`,`time_prc`,`ip`,`agent`) values (arg_uid,arg_txt,NOW(),arg_ip,arg_agent);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `verifyEmail`
-- ----------------------------
DROP PROCEDURE IF EXISTS `verifyEmail`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `verifyEmail`(in arg_did char(16),in arg_code char(32))
BEGIN
    set @uid = (select `uid` from `email_verify_codes` where `code`=arg_code);
    set @n = (select count(@uid));
    if @n > 0 then
        set @did = (select `did` from `users_stu` where `id`=@uid);
        if (@did = arg_did) then
            delete from `email_verify_codes` where `uid`=@uid;
            set @step = (select `step` from `users_stu` where `id`=@uid);
            if @step=1 then
                update `users_stu` set `email_validated`=1,`step`=2 where `id`=@uid;
            else
                update `users_stu` set `email_validated`=1 where `id`=@uid;
            end if;
            select @uid;
        end if;
    end if;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `visit`
-- ----------------------------
DROP PROCEDURE IF EXISTS `visit`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `visit`(in arg_did char(16), in arg_uid bigint, in arg_ip varchar(40), in arg_agent varchar(255))
BEGIN
    set time_zone = '+8:00';
    set @stuid = (select stuid from si_stu where did=arg_did);
    if @stuid is not null then
        insert into log_visit_stu (uid,stuid,time_prc,ip,agent) values (arg_uid,@stuid,NOW(),arg_ip,arg_agent);
    end if;
    select name,sex,birthday,native,nation,home_address,zone,classname,sum(r.score),count(r.score) from si_stu u inner join si_class c on c.classid=u.class left join ratings r on r.stuid=@stuid where u.did=arg_did;
END
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `says_replies_increment`;
DELIMITER ;;
CREATE TRIGGER `says_replies_increment` AFTER INSERT ON `says_replies` FOR EACH ROW begin
    update `says` set `replies_count`=`replies_count`+1 where `says_id`=new.says_id;
end
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `says_replies_decrement`;
DELIMITER ;;
CREATE TRIGGER `says_replies_decrement` AFTER DELETE ON `says_replies` FOR EACH ROW begin
    update `says` set `replies_count`=`replies_count`-1 where `says_id`=old.says_id;
end
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `loggedin_decrease`;
DELIMITER ;;
CREATE TRIGGER `loggedin_decrease` AFTER DELETE ON `sessions` FOR EACH ROW BEGIN
      set @n = (select count(*) from `sessions` where `uid`=old.uid);
      update `users_stu` set `loggedin`=@n where `id`=old.uid;
      insert into `log_login_stu` (`uid`,`time_prc`,`logout`) values (old.uid,NOW(),1);
END
;;
DELIMITER ;
