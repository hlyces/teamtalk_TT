/*
SQLyog v10.2 
MySQL - 5.5.5-10.0.19-MariaDB-wsrep-log : Database - teamtalk
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
USE `teamtalk`;

/*!50106 set global event_scheduler = 1*/;

/* Event structure for event `schedule_DeleteMsgAndFileLog_daily` */

/*!50106 DROP EVENT IF EXISTS `schedule_DeleteMsgAndFileLog_daily`*/;

DELIMITER $$

/*!50106 CREATE DEFINER=`root`@`%` EVENT `schedule_DeleteMsgAndFileLog_daily` ON SCHEDULE EVERY 1 DAY STARTS '2015-08-20 00:00:00' ON COMPLETION NOT PRESERVE ENABLE DO BEGIN
	    CALL proc_DeleteMsgAndFileLog_daily();
	END */$$
DELIMITER ;

/* Procedure structure for procedure `proc_DeleteMsgAndFileLog_daily` */

/*!50003 DROP PROCEDURE IF EXISTS  `proc_DeleteMsgAndFileLog_daily` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `proc_DeleteMsgAndFileLog_daily`()
BEGIN
declare interval_time int default 30*24*3600; #清除30天前的消息数据
DELETE FROM IMMessage_0 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_1 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_2 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_3 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_4 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_5 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_6 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMMessage_7 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_0 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_1 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_2 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_3 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_4 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_5 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_6 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
DELETE FROM IMGroupMessage_7 WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
set interval_time=7*24*3600; #清除7天前的文件消息数据
DELETE FROM IMTransmitFile WHERE created<UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-interval_time;
    END */$$
DELIMITER ;

/*Table structure for table `IMFxAccount` */

DROP TABLE IF EXISTS `IMFxAccount`;

/*!50001 DROP VIEW IF EXISTS `IMFxAccount` */;
/*!50001 DROP TABLE IF EXISTS `IMFxAccount` */;

/*!50001 CREATE TABLE  `IMFxAccount`(
 `id` int(11) ,
 `user_account` varchar(32) ,
 `user_password` varchar(32) ,
 `user_salt` int(11) ,
 `user_phone` varchar(11) ,
 `user_blacksign` bigint(20) 
)*/;

/*View structure for view IMFxAccount */

/*!50001 DROP TABLE IF EXISTS `IMFxAccount` */;
/*!50001 DROP VIEW IF EXISTS `IMFxAccount` */;

/*!50001 CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`%` SQL SECURITY DEFINER VIEW `IMFxAccount` AS select `t1`.`id` AS `id`,`t1`.`user_account` AS `user_account`,`t1`.`user_password` AS `user_password`,`t1`.`user_salt` AS `user_salt`,`t1`.`user_phone` AS `user_phone`,0 AS `user_blacksign` from `teamtalk`.`fx_account2` `t1` union all select `t2`.`uid` AS `id`,`t2`.`uid` AS `user_account`,md5(`t2`.`user_password`) AS `user_password`,`t2`.`user_salt` AS `user_salt`,`t2`.`user_phone` AS `user_phone`,0 AS `user_blacksign` from `dffx_db_bs`.`dffx_user_account` `t2` */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
