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
   user_uid             INT(11) NOT NULL COMMENT '�û��˺�',
   user_nickname        VARCHAR(20) COMMENT '�ǳ�',
   user_sex             INT(11) DEFAULT 1 COMMENT '�Ա�',
   user_birthday        BIGINT COMMENT '����',
   user_headlink        VARCHAR(200) COMMENT 'ͷ��',
   user_level           SMALLINT(6) COMMENT '�ȼ�',
   user_regedittime     INT(11) NOT NULL DEFAULT 0,
   user_updatetime      INT(11) NOT NULL DEFAULT 0,
   STATUS               TINYINT(4) DEFAULT 0,
   user_status          TINYINT(4) DEFAULT 2 COMMENT '����״̬,1=���ߣ�2=���ߣ�3=�뿪',
   user_phone           VARCHAR(20) NOT NULL COMMENT '�ֻ�����',
   user_type            TINYINT(4) NOT NULL DEFAULT 0 COMMENT '�û�����',
   user_ischeck         SMALLINT(6) COMMENT '�û���֤״̬(0:δ��֤1:������֤ 2:����֤)',
   user_desc            VARCHAR(400) COMMENT '������ǩ��',
   pc_status            TINYINT NOT NULL DEFAULT 2 COMMENT 'PC����״̬',
   mobile_status        TINYINT NOT NULL DEFAULT 2 COMMENT '�ƶ�������״̬',
   pc_msgserver_id      INT COMMENT 'PC�˵�msgserver��id',
   mobile_msgserver_id  INT COMMENT '�ƶ��˵�msgserver��id',
   pc_logintime         INT COMMENT 'pc�˵�¼ʱ��',
   mobile_logintime     INT COMMENT '�ƶ��˵�¼ʱ��',
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

