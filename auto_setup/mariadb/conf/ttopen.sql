/*==============================================================*/
/* Database name:  dffx_db_im                                     */
/* DBMS name:      MySQL 5.0                                    */
/* Created on:     2015/8/14 16:58:47                           */
/*==============================================================*/


drop database if exists dffx_db_im ;

/*==============================================================*/
/* Database: dffx_db_im                                           */
/*==============================================================*/
create database dffx_db_im DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

use dffx_db_im;

/*==============================================================*/
/* Table: IMAdmin                                               */
/*==============================================================*/
create table IMAdmin
(
   id                   mediumint(6) unsigned not null auto_increment,
   uname                varchar(40) not null comment '用户名',
   pwd                  char(32) not null comment '密码',
   status               tinyint(2) unsigned not null default 0 comment '用户状态 0 :正常 1:删除 可扩展',
   created              int(11) unsigned not null default 0 comment '创建时间',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   primary key (id)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMAudio                                               */
/*==============================================================*/
create table IMAudio
(
   id                   int(11) not null auto_increment,
   fromId               int(11) unsigned not null comment '发送者Id',
   toId                 int(11) unsigned not null comment '接收者Id',
   path                 varchar(255),
   size                 int(11) unsigned not null default 0 comment '文件大小',
   duration             int(11) unsigned not null default 0 comment '语音时长',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_fromId_toId (fromId, toId)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMDepart                                              */
/*==============================================================*/
create table IMDepart
(
   id                   int(11) unsigned not null auto_increment comment '部门id',
   departName           varchar(64) not null,
   priority             int(11) unsigned not null default 0 comment '显示优先级',
   parentId             int(11) unsigned not null comment '上级部门id',
   status               int(11) unsigned not null default 0 comment '状态',
   created              int(11) unsigned not null comment '创建时间',
   updated              int(11) unsigned not null comment '更新时间',
   primary key (id),
   key idx_departName (departName),
   key idx_priority_status (priority, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMDiscovery                                           */
/*==============================================================*/
create table IMDiscovery
(
   id                   int(11) unsigned not null auto_increment comment 'id',
   itemName             varchar(64) not null,
   itemUrl              varchar(64),
   itemPriority         int(11) unsigned not null comment '显示优先级',
   status               int(11) unsigned not null default 0 comment '状态',
   created              int(11) unsigned not null comment '创建时间',
   updated              int(11) unsigned not null comment '更新时间',
   primary key (id),
   key idx_itemName (itemName)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMFxUser                                              */
/*==============================================================*/
create table IMFxUser
(
   id                   int(20) unsigned not null auto_increment,
   user_uid             int(11) not null COMMENT '用户账号',
   user_nickname        varchar(20) default NULL COMMENT '昵称',
   user_sex             int(11) default 1 COMMENT '性别',
   user_birthday        bigint default NULL COMMENT '生日',
   user_headlink        varchar(200) default NULL COMMENT '头像',
   user_level           smallint(6) default NULL COMMENT '等级',
   user_regedittime     int(11) not null default 0,
   user_updatetime      int(11) not null default 0,
   status               tinyint(4) default 0,
   user_status          tinyint(4) default 2 COMMENT '在线状态1=在线, 2=离线',
   user_phone           varchar(20) not null COMMENT '手机号码',
   user_type            tinyint(4) not null default 0 COMMENT '用户类型',
   user_ischeck		smallint(6) not null DEFAULT 0 COMMENT '用户认证状态(0:未认证1:申请认证 2:已认证)',
   user_desc            varchar(400) comment '描述或签名',
   primary key (id),
   unique key idx_useruid (user_uid)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMFxUserRelationGroup                                 */
/*==============================================================*/
create table IMFxUserRelationGroup
(
   id                   int(11) not null auto_increment,
   group_id             int(11) not null,
   group_name           varchar(20) default NULL,
   user_uid             int(11) not null default 0,
   flag                 tinyint(4) not null default 0,
   sort                 int(11) not null default 1,
   primary key (id),
   unique key idx_groupid_userid (group_id, user_uid),
   key fk_group_user_uid (user_uid)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMFxUserRelationPrivate                               */
/*==============================================================*/
create table IMFxUserRelationPrivate
(
   id                   int(11) not null auto_increment,
   user_uid             int(11) not null default 0,
   friend_friendid      int(11) not null default 0,
   friend_groupid       int(11) default NULL,
   friend_remark        varchar(20) default NULL,
   status               tinyint(4) not null default 0,
   extra_info           varchar(50) default NULL,
   update_time          int(10) unsigned not null default 1,
   primary key (id),
   unique key idx_userid_friend (user_uid, friend_friendid),
   key fk_private_friend_uid (friend_friendid)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMGroup                                               */
/*==============================================================*/
create table IMGroup
(
   id                   int(11) not null auto_increment,
   name                 varchar(256) not null,
   avatar               varchar(256),
   creator              int(11) unsigned not null default 0 comment '创建者用户id',
   type                 tinyint(3) unsigned not null default 1 comment '群组类型，1-固定;2-临时群',
   userCnt              int(11) unsigned not null default 0 comment '成员人数',
   status               tinyint(3) unsigned not null default 1 comment '是否删除,0-正常，1-删除',
   version              int(11) unsigned not null default 1 comment '群版本号',
   lastChated           int(11) unsigned not null default 0 comment '最后聊天时间',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_name (name),
   key idx_creator (creator)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群信息';

/*==============================================================*/
/* Table: IMGroupMember                                         */
/*==============================================================*/
create table IMGroupMember
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '群Id',
   userId               int(11) unsigned not null comment '用户id',
   status               tinyint(4) unsigned not null default 1 comment '是否退出群，0-正常，1-已退出',
   created              int(11) unsigned not null default 0 comment '创建时间',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   primary key (id),
   key idx_groupId_userId_status (groupId, userId, status),
   key idx_userId_status_updated (userId, status, updated),
   key idx_groupId_updated (groupId, updated)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMGroupMessage_0                                      */
/*==============================================================*/
create table IMGroupMessage_0
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_1                                      */
/*==============================================================*/
create table IMGroupMessage_1
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_2                                      */
/*==============================================================*/
create table IMGroupMessage_2
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_3                                      */
/*==============================================================*/
create table IMGroupMessage_3
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_4                                      */
/*==============================================================*/
create table IMGroupMessage_4
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_5                                      */
/*==============================================================*/
create table IMGroupMessage_5
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_6                                      */
/*==============================================================*/
create table IMGroupMessage_6
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMGroupMessage_7                                      */
/*==============================================================*/
create table IMGroupMessage_7
(
   id                   int(11) not null auto_increment,
   groupId              int(11) unsigned not null comment '用户的关系id',
   userId               int(11) unsigned not null comment '发送用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(3) unsigned not null default 2 comment '群消息类型,101为群语音,2为文本',
   status               int(11) unsigned not null default 0 comment '消息状态',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   created              int(11) unsigned not null default 0 comment '创建时间',
   primary key (id),
   key idx_groupId_status_created (groupId, status, created),
   key idx_groupId_msgId_status_created (groupId, msgId, status, created)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='IM群消息表';

/*==============================================================*/
/* Table: IMMessage_0                                           */
/*==============================================================*/
create table IMMessage_0
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_1                                           */
/*==============================================================*/
create table IMMessage_1
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_2                                           */
/*==============================================================*/
create table IMMessage_2
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_3                                           */
/*==============================================================*/
create table IMMessage_3
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_4                                           */
/*==============================================================*/
create table IMMessage_4
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_5                                           */
/*==============================================================*/
create table IMMessage_5
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_6                                           */
/*==============================================================*/
create table IMMessage_6
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMMessage_7                                           */
/*==============================================================*/
create table IMMessage_7
(
   id                   int(11) not null auto_increment,
   relateId             int(11) unsigned not null comment '用户的关系id',
   fromId               int(11) unsigned not null comment '发送用户的id',
   toId                 int(11) unsigned not null comment '接收用户的id',
   msgId                int(11) unsigned not null comment '消息ID',
   content              varchar(2048),
   type                 tinyint(2) unsigned not null default 1 comment '消息类型',
   status               tinyint(1) unsigned not null default 0 comment '0正常 1被删除',
   updated              int(11) unsigned not null comment '更新时间',
   created              int(11) unsigned not null comment '创建时间',
   primary key (id),
   key idx_relateId_status_created (relateId, status, created),
   key idx_relateId_status_msgId_created (relateId, status, msgId, created),
   key idx_fromId_toId_created (fromId, toId, status)
)
ENGINE=NDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

/*==============================================================*/
/* Table: IMRecentSession                                       */
/*==============================================================*/
create table IMRecentSession
(
   id                   int(11) not null auto_increment,
   userId               int(11) unsigned not null comment '用户id',
   peerId               int(11) unsigned not null comment '对方id',
   type                 tinyint(1) unsigned not null default 0 comment '类型，1-用户,2-群组',
   status               tinyint(1) unsigned not null default 0 comment '用户:0-正常, 1-用户A删除,群组:0-正常, 1-被删除',
   created              int(11) unsigned not null default 0 comment '创建时间',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   primary key (id),
   key idx_userId_peerId_status_updated (userId, peerId, status, updated),
   key idx_userId_peerId_type (userId, peerId, type)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMRelationShip                                        */
/*==============================================================*/
create table IMRelationShip
(
   id                   int(11) not null auto_increment,
   smallId              int(11) unsigned not null comment '用户A的id',
   bigId                int(11) unsigned not null comment '用户B的id',
   status               tinyint(1) unsigned not null default 0 comment '用户:0-正常, 1-用户A删除,群组:0-正常, 1-被删除',
   created              int(11) unsigned not null default 0 comment '创建时间',
   updated              int(11) unsigned not null default 0 comment '更新时间',
   primary key (id),
   key idx_smallId_bigId_status_updated (smallId, bigId, status, updated)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: IMTransmitFile                                        */
/*==============================================================*/
create table IMTransmitFile
(
   id                   int(10) unsigned not null auto_increment,
   fromId               int(11) not null,
   toId                 int(11) not null,
   fileName             varchar(256) not null,
   fileSize             int(11) not null,
   taskId               varchar(50) not null,
   status               tinyint(4) not null default 0,
   created              int(11) not null default 1,
   updated              int(11) not null default 1,
   ipAddr               varchar(256),
   primary key (id)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

/*==============================================================*/
/* Table: fx_account2                                           */
/*==============================================================*/
create table fx_account2
(
   id                   int(11) not null auto_increment,
   user_account         varchar(32) default NULL,
   user_password        varchar(32) default NULL,
   user_salt            int(11) default NULL,
   user_phone           varchar(11) default NULL,
   user_lasttime        timestamp not null default CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
   user_lastaddress     varchar(20) default NULL,
   user_token           varchar(20) default NULL,
   primary key (id)
)
ENGINE = NDB
DEFAULT CHARSET = utf8mb4
COLLATE = utf8mb4_bin;

alter table IMFxUserRelationGroup add constraint fk_group_user_uid foreign key (user_uid)
      references IMFxUser (user_uid) on delete cascade;

alter table IMFxUserRelationPrivate add constraint fk_private_friend_uid foreign key (friend_friendid)
      references IMFxUser (user_uid) on delete cascade;

alter table IMFxUserRelationPrivate add constraint fk_private_user_uid foreign key (user_uid)
      references IMFxUser (user_uid) on delete cascade;

