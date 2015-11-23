/*==============================================================*/
/* Database name:  dffx_db_im                                   */
/* DBMS name:      MySQL 5.0                                    */
/* Created on:     2015/11/10 17:36:14                          */
/*==============================================================*/


USE dffx_db_im;

DROP TABLE IF EXISTS tmp_IMFxUser;

RENAME TABLE IMFxUser TO tmp_IMFxUser;


/*==============================================================*/
/* Table: IMFxUser                                              */
/*==============================================================*/
CREATE TABLE dffx_db_im.IMFxUser
(
   id                   INT NOT NULL AUTO_INCREMENT,
   user_uid             INT(11) NOT NULL COMMENT '用户账号',
   user_nickname        VARCHAR(20) COMMENT '昵称',
   user_sex             INT(11) DEFAULT 1 COMMENT '性别',
   user_birthday        BIGINT COMMENT '生日',
   user_headlink        VARCHAR(200) COMMENT '头像',
   user_level           SMALLINT(6) COMMENT '等级',
   user_regedittime     INT(11) NOT NULL DEFAULT 0,
   user_updatetime      INT(11) NOT NULL DEFAULT 0,
   STATUS               TINYINT(4) DEFAULT 0,
   user_status          TINYINT(4) DEFAULT 2 COMMENT '在线状态,1=在线，2=离线，3=离开',
   user_phone           VARCHAR(20) NOT NULL COMMENT '手机号码',
   user_type            TINYINT(4) NOT NULL DEFAULT 0 COMMENT '用户类型',
   user_ischeck         SMALLINT(6) COMMENT '用户认证状态(0:未认证1:申请认证 2:已认证)',
   user_desc            VARCHAR(400) COMMENT '描述或签名',
   pc_status            TINYINT NOT NULL DEFAULT 2 COMMENT 'PC在线状态',
   mobile_status        TINYINT NOT NULL DEFAULT 2 COMMENT '移动端在线状态',
   pc_msgserver_id      INT COMMENT 'PC端的msgserver的id',
   mobile_msgserver_id  INT COMMENT '移动端的msgserver的id',
   pc_logintime         INT COMMENT 'pc端登录时间',
   mobile_logintime     INT COMMENT '移动端登录时间',
   PRIMARY KEY (id),
   KEY idx_useruid (user_uid)
)
ENGINE = INNODB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

INSERT INTO dffx_db_im.IMFxUser (id, user_uid, user_nickname, user_sex, user_birthday, user_headlink, user_level, user_regedittime, user_updatetime, STATUS, user_status, user_phone, user_type, user_ischeck, user_desc)
SELECT id, user_uid, user_nickname, user_sex, user_birthday, user_headlink, user_level, user_regedittime, user_updatetime, STATUS, user_status, user_phone, user_type, user_ischeck, user_desc
FROM tmp_IMFxUser;

SET FOREIGN_KEY_CHECKS=0;
DROP TABLE IF EXISTS tmp_IMFxUser;
SET FOREIGN_KEY_CHECKS=1;

ALTER TABLE IMFxUserRelationGroup ADD CONSTRAINT fk_group_user_uid FOREIGN KEY (user_uid)
      REFERENCES dffx_db_im.IMFxUser (user_uid) ON DELETE CASCADE;

ALTER TABLE IMFxUserRelationPrivate ADD CONSTRAINT fk_private_friend_uid FOREIGN KEY (friend_friendid)
      REFERENCES dffx_db_im.IMFxUser (user_uid) ON DELETE CASCADE;

ALTER TABLE IMFxUserRelationPrivate ADD CONSTRAINT fk_private_user_uid FOREIGN KEY (user_uid)
      REFERENCES dffx_db_im.IMFxUser (user_uid) ON DELETE CASCADE;

