drop table dffx_user_check;
CREATE TABLE `dffx_user_check` (
	`id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'ÓÃ»§ID',
  `check_uid` int(11) NOT NULL COMMENT '用户账号(UID)',
  `check_status` smallint(6) DEFAULT NULL COMMENT '用户认证状态(0：认证成功, 1：认证失败)',
	PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1969 DEFAULT CHARSET=utf8 COMMENT='用户认证表';