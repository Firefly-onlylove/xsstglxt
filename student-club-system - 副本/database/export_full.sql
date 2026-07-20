
-- ============================================================
-- SCMS 数据库完整导出（结构 + 数据 + 触发器 + 存储过程）
-- 导出时间：2026-07-19
-- 源数据库：scms_db (MySQL 8.0)
--
-- 对方导入方式：
--   mysql -u root -p -e "DROP DATABASE IF EXISTS scms_db; CREATE DATABASE scms_db DEFAULT CHARSET utf8mb4;"
--   mysql -u root -p --default-character-set=utf8mb4 scms_db < export_full.sql
--
-- ⚠ 导入前请备份自己的 scms_db 数据
-- ============================================================

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `activities`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `activities` (
  `activity_id` int NOT NULL AUTO_INCREMENT,
  `club_id` int NOT NULL,
  `title` varchar(200) NOT NULL,
  `description` text,
  `location` varchar(200) DEFAULT NULL,
  `start_time` datetime NOT NULL,
  `end_time` datetime NOT NULL,
  `signup_deadline` datetime DEFAULT NULL,
  `max_participants` int DEFAULT '0',
  `current_count` int DEFAULT '0',
  `enable_signin` tinyint NOT NULL DEFAULT '0',
  `checkin_code` varchar(6) DEFAULT NULL,
  `require_summary` tinyint NOT NULL DEFAULT '0',
  `summary` text,
  `status` enum('draft','published','ongoing','completed','cancelled','archived') NOT NULL DEFAULT 'draft',
  `cancel_reason` varchar(255) DEFAULT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `join_permission` varchar(16) NOT NULL DEFAULT 'all' COMMENT '鎶ュ悕鏉冮檺: all=鍏ㄩ儴鍙?姤, members_only=浠呮湰绀炬垚鍛',
  PRIMARY KEY (`activity_id`),
  KEY `idx_club` (`club_id`),
  KEY `idx_status` (`status`),
  KEY `idx_start` (`start_time`),
  CONSTRAINT `activities_ibfk_1` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `activities` WRITE;
/*!40000 ALTER TABLE `activities` DISABLE KEYS */;
INSERT INTO `activities` VALUES (2,1,'Test Activity','Test Desc','Room 101','2026-08-15 14:00:00','2026-08-15 16:00:00','2026-08-10 23:59:59',50,0,0,'645006',0,NULL,'completed',NULL,'2026-07-18 16:50:59','2026-07-18 16:51:20','all'),(3,1,'Updated Draft','','','2026-09-01 14:00:00','2026-09-01 16:00:00','2026-08-25 23:59:59',100,0,0,'043434',0,NULL,'completed',NULL,'2026-07-18 16:51:10','2026-07-18 16:51:38','all'),(4,1,'SQL直接测试','desc','loc','2026-09-01 10:00:00','2026-09-01 12:00:00','2026-08-28 23:59:59',60,25,0,'587719',0,NULL,'published',NULL,'2026-07-18 16:56:02','2026-07-19 10:54:19','all'),(5,1,'Full Test','test','Room','2026-09-01 10:00:00','2026-09-01 12:00:00','2026-08-28 23:59:59',50,0,0,'346257',0,NULL,'ongoing',NULL,'2026-07-18 16:56:28','2026-07-18 21:51:45','all'),(6,1,'Missing Signup','test','Room','2026-09-01 10:00:00','2026-09-01 12:00:00',NULL,50,1,0,'262441',0,'Activity went well','published',NULL,'2026-07-18 16:56:28','2026-07-18 17:25:57','all'),(7,1,'Demo Activity','A demo activity','Room 101','2026-09-15 10:00:00','1970-01-01 00:00:00','1970-01-01 00:00:00',30,0,0,'458449',0,NULL,'ongoing',NULL,'2026-07-18 17:25:12','2026-07-18 17:25:39','all'),(8,1,'Signin Test Activity','Testing signin flow','Room 202','2026-09-20 10:00:00','1970-01-01 00:00:00','1970-01-01 00:00:00',20,4,0,'973767',0,NULL,'published',NULL,'2026-07-18 17:25:38','2026-07-18 20:55:49','all'),(9,1,'Signin Flow Test','Testing complete signin','Room 303','2026-10-01 09:00:00','1970-01-01 00:00:00','1970-01-01 00:00:00',30,0,0,'383875',0,NULL,'published',NULL,'2026-07-18 17:27:31','2026-07-18 21:51:26','all'),(10,1,'123','1','1','2026-07-18 22:20:05','2026-07-25 00:00:00','2026-07-22 00:00:00',67,0,0,'327149',0,NULL,'ongoing',NULL,'2026-07-18 22:20:24','2026-07-18 22:25:26','all'),(11,1,'test2','test','Room','2026-10-01 10:00:00','1970-01-01 00:00:00','1970-01-01 00:00:00',0,0,1,'352203',0,NULL,'published',NULL,'2026-07-18 22:47:01','2026-07-18 22:47:44','members_only'),(12,1,'NewTest','test','Room101','2026-10-01 10:00:00','1970-01-01 00:00:00','1970-01-01 00:00:00',100,0,0,NULL,0,NULL,'draft',NULL,'2026-07-18 23:13:30','2026-07-18 23:13:30','all'),(13,1,'test default max 50','test','test','2026-11-01 10:00:00','1970-01-01 00:00:00','1970-01-01 00:00:00',50,2,0,NULL,0,NULL,'published',NULL,'2026-07-18 23:15:06','2026-07-19 10:54:45','all');
/*!40000 ALTER TABLE `activities` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `activity_signins`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `activity_signins` (
  `signin_id` int NOT NULL AUTO_INCREMENT,
  `activity_id` int NOT NULL,
  `user_id` int NOT NULL,
  `signin_method` enum('code','manual') NOT NULL,
  `signin_time` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`signin_id`),
  UNIQUE KEY `uk_signin` (`activity_id`,`user_id`),
  KEY `user_id` (`user_id`),
  KEY `idx_activity` (`activity_id`),
  CONSTRAINT `activity_signins_ibfk_1` FOREIGN KEY (`activity_id`) REFERENCES `activities` (`activity_id`) ON DELETE CASCADE,
  CONSTRAINT `activity_signins_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `activity_signins` WRITE;
/*!40000 ALTER TABLE `activity_signins` DISABLE KEYS */;
/*!40000 ALTER TABLE `activity_signins` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `announcements`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `announcements` (
  `announcement_id` int NOT NULL AUTO_INCREMENT,
  `title` varchar(200) NOT NULL,
  `content` text NOT NULL,
  `scope` enum('school','college','club') NOT NULL,
  `college_id` int DEFAULT NULL,
  `club_id` int DEFAULT NULL,
  `publisher_id` int NOT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`announcement_id`),
  KEY `college_id` (`college_id`),
  KEY `club_id` (`club_id`),
  KEY `publisher_id` (`publisher_id`),
  KEY `idx_scope` (`scope`),
  KEY `idx_time` (`created_at`),
  CONSTRAINT `announcements_ibfk_1` FOREIGN KEY (`college_id`) REFERENCES `colleges` (`college_id`) ON DELETE CASCADE,
  CONSTRAINT `announcements_ibfk_2` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE,
  CONSTRAINT `announcements_ibfk_3` FOREIGN KEY (`publisher_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `announcements` WRITE;
/*!40000 ALTER TABLE `announcements` DISABLE KEYS */;
INSERT INTO `announcements` VALUES (2,'2026年度优秀社团评选通知','为了表彰在2025-2026学年中表现突出的学生社团，学校决定开展年度优秀社团评选活动。评选标准包括社团活动质量、成员发展、财务管理等多个维度。请各社团在8月30日前提交申报材料。','school',NULL,NULL,1,'2026-07-18 16:28:22'),(3,'信科院社团活动场地使用新规定','为进一步规范学院实验室使用，即日起信科院所属社团借用实验室场地需至少提前3天申请，并在活动后当天完成场地清理。违者将暂停场地使用权限。','college',1,NULL,2,'2026-07-18 16:28:22'),(4,'文学院社团经费调整通知','经学院研究决定，自2026年秋季学期起，文学院社团经费标准调整为：普通社团每学期1000元，重点社团每学期2000元。请各社团根据新标准调整预算。','college',2,NULL,3,'2026-07-18 16:28:22'),(5,'ACM编程协会新社员招募中','ACM编程协会欢迎所有对算法和编程感兴趣的同学加入！零基础也可，我们提供系统化的算法培训课程。报名方式：在社团广场搜索\"ACM编程协会\"并提交入社申请。','club',NULL,1,101,'2026-07-18 16:28:22'),(6,'绿茵足球社暑期训练计划','暑期足球训练将于7月至8月每周二、四下午4:00-6:00在北校区足球场进行，请各位社员安排好时间，积极参加。','club',NULL,4,108,'2026-07-18 16:28:22'),(7,'SQL测试','内容','school',NULL,NULL,1,'2026-07-18 17:05:37'),(8,'Test','Test content','school',NULL,NULL,1,'2026-07-18 17:12:31'),(9,'abc','def','school',NULL,NULL,1,'2026-07-18 17:12:58'),(10,'a','b','school',NULL,NULL,1,'2026-07-18 17:13:02'),(11,'a\'b','c','school',NULL,NULL,1,'2026-07-18 17:13:02'),(12,'测试公告','测试内容','school',NULL,NULL,1,'2026-07-18 17:17:43'),(13,'test??','test','school',NULL,NULL,1,'2026-07-18 17:17:51'),(14,'2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:04'),(15,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:05'),(16,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:05'),(17,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:05'),(18,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:06'),(19,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:07'),(20,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:08'),(21,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:08'),(22,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:09'),(23,'2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a2a','test','school',NULL,NULL,1,'2026-07-18 17:18:10'),(24,'test','xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx','school',NULL,NULL,1,'2026-07-18 17:18:31'),(25,'test','xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx','school',NULL,NULL,1,'2026-07-18 17:18:31'),(26,'test','xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx','school',NULL,NULL,1,'2026-07-18 17:18:31'),(27,'test','xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx','school',NULL,NULL,1,'2026-07-18 17:18:31'),(28,'test','xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx','school',NULL,NULL,1,'2026-07-18 17:18:32'),(29,'中文标题','中文内容','school',NULL,NULL,1,'2026-07-18 17:18:49'),(30,'System Notice','Please check the new semester schedule','school',NULL,NULL,1,'2026-07-18 17:23:09'),(31,'CS College Notice','Important notice for CS college','college',1,NULL,2,'2026-07-18 17:24:50');
/*!40000 ALTER TABLE `announcements` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `backup_records`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `backup_records` (
  `backup_id` int NOT NULL AUTO_INCREMENT,
  `backup_type` enum('auto','manual') NOT NULL,
  `file_path` varchar(500) NOT NULL,
  `file_size` bigint DEFAULT NULL,
  `operator_id` int DEFAULT NULL,
  `status` enum('success','failed') NOT NULL DEFAULT 'success',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`backup_id`),
  KEY `operator_id` (`operator_id`),
  KEY `idx_time` (`created_at`),
  CONSTRAINT `backup_records_ibfk_1` FOREIGN KEY (`operator_id`) REFERENCES `users` (`user_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `backup_records` WRITE;
/*!40000 ALTER TABLE `backup_records` DISABLE KEYS */;
INSERT INTO `backup_records` VALUES (1,'manual','storage/backups/scms_2026_07_18_16_45_21.sql',0,1,'failed','2026-07-18 16:45:21'),(2,'manual','storage/backups/scms_2026_07_18_16_55_14.sql',0,1,'failed','2026-07-18 16:55:14'),(3,'manual','storage/backups/scms_2026_07_18_17_28_33.sql',0,1,'failed','2026-07-18 17:28:33');
/*!40000 ALTER TABLE `backup_records` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `classes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `classes` (
  `class_id` int NOT NULL AUTO_INCREMENT,
  `major_id` int NOT NULL,
  `college_id` int NOT NULL,
  `class_name` varchar(100) NOT NULL,
  `grade` varchar(10) DEFAULT NULL,
  `status` tinyint NOT NULL DEFAULT '1',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`class_id`),
  UNIQUE KEY `uk_major_class` (`major_id`,`class_name`),
  KEY `college_id` (`college_id`),
  CONSTRAINT `classes_ibfk_1` FOREIGN KEY (`major_id`) REFERENCES `majors` (`major_id`) ON DELETE CASCADE,
  CONSTRAINT `classes_ibfk_2` FOREIGN KEY (`college_id`) REFERENCES `colleges` (`college_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `classes` WRITE;
/*!40000 ALTER TABLE `classes` DISABLE KEYS */;
INSERT INTO `classes` VALUES (1,1,1,'计科2301班','2023',1,'2026-07-17 20:52:14'),(2,1,1,'计科2401班','2024',1,'2026-07-17 20:52:14'),(3,2,1,'软工2301班','2023',1,'2026-07-17 20:52:14'),(4,2,1,'软工2401班','2024',1,'2026-07-17 20:52:14'),(5,3,1,'大数据2301班','2023',1,'2026-07-17 20:52:14'),(6,3,1,'大数据2401班','2024',1,'2026-07-17 20:52:14'),(7,4,1,'人工智能2301班','2023',1,'2026-07-17 20:52:14'),(8,4,1,'人工智能2401班','2024',1,'2026-07-17 20:52:14'),(9,5,2,'汉语言2301班','2023',1,'2026-07-17 20:52:14'),(10,5,2,'汉语言2401班','2024',1,'2026-07-17 20:52:14'),(11,6,2,'新闻2301班','2023',1,'2026-07-17 20:52:14'),(12,7,2,'广告2301班','2023',1,'2026-07-17 20:52:14'),(13,8,3,'数学2301班','2023',1,'2026-07-17 20:52:14'),(14,9,3,'物理2301班','2023',1,'2026-07-17 20:52:14'),(15,10,3,'化学2301班','2023',1,'2026-07-17 20:52:14'),(16,11,4,'土木2301班','2023',1,'2026-07-17 20:52:14'),(17,12,4,'机械2301班','2023',1,'2026-07-17 20:52:14'),(18,13,4,'电气2301班','2023',1,'2026-07-17 20:52:14'),(19,14,5,'经济2301班','2023',1,'2026-07-17 20:52:14'),(20,15,5,'工商2301班','2023',1,'2026-07-17 20:52:14'),(21,16,5,'会计2301班','2023',1,'2026-07-17 20:52:14'),(22,17,6,'临床2301班','2023',1,'2026-07-17 20:52:14'),(23,18,6,'护理2301班','2023',1,'2026-07-17 20:52:14'),(24,19,6,'药学2301班','2023',1,'2026-07-17 20:52:14');
/*!40000 ALTER TABLE `classes` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `clubs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `clubs` (
  `club_id` int NOT NULL AUTO_INCREMENT,
  `club_name` varchar(100) NOT NULL,
  `description` text,
  `category` varchar(50) DEFAULT NULL,
  `logo_path` varchar(255) DEFAULT NULL,
  `level` enum('school','college') NOT NULL DEFAULT 'college',
  `college_id` int DEFAULT NULL,
  `president_id` int DEFAULT NULL,
  `vice_president_count` int DEFAULT '1',
  `advisor` varchar(50) DEFAULT NULL,
  `member_count` int DEFAULT '0',
  `join_permission` enum('all','college_only') NOT NULL DEFAULT 'all',
  `status` enum('pending','approved','rejected','archived','dissolved') NOT NULL DEFAULT 'pending',
  `reject_reason` varchar(255) DEFAULT NULL,
  `created_by` int DEFAULT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`club_id`),
  UNIQUE KEY `club_name` (`club_name`),
  KEY `president_id` (`president_id`),
  KEY `created_by` (`created_by`),
  KEY `idx_level` (`level`),
  KEY `idx_status` (`status`),
  KEY `idx_category` (`category`),
  KEY `idx_college` (`college_id`),
  CONSTRAINT `clubs_ibfk_1` FOREIGN KEY (`college_id`) REFERENCES `colleges` (`college_id`) ON DELETE SET NULL,
  CONSTRAINT `clubs_ibfk_2` FOREIGN KEY (`president_id`) REFERENCES `users` (`user_id`) ON DELETE SET NULL,
  CONSTRAINT `clubs_ibfk_3` FOREIGN KEY (`created_by`) REFERENCES `users` (`user_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `clubs` WRITE;
/*!40000 ALTER TABLE `clubs` DISABLE KEYS */;
INSERT INTO `clubs` VALUES (1,'ACM编程协会','面向全校的算法竞赛与编程交流社团，定期举办算法讲座和ACM训练赛。','学术',NULL,'school',1,203,2,'陈老师',25,'all','approved',NULL,101,'2026-07-17 20:52:14','2026-07-18 21:34:20'),(2,'志愿者协会','组织校内外公益志愿活动，传递爱心与正能量。','公益',NULL,'school',NULL,102,2,'林老师',14,'all','approved',NULL,102,'2026-07-17 20:52:14','2026-07-18 16:31:26'),(3,'星空天文社','仰望星空，探索宇宙奥秘，组织观星活动与天文科普。','学术',NULL,'school',NULL,106,1,'黄老师',9,'all','approved',NULL,106,'2026-07-17 20:52:14','2026-07-18 16:27:11'),(4,'绿茵足球社','热爱足球的同学的大家庭，日常训练与院际联赛。','体育',NULL,'school',NULL,108,2,'刘老师',10,'all','approved',NULL,108,'2026-07-17 20:52:14','2026-07-18 16:27:11'),(5,'信科算法社','信息科学与技术学院内部算法学习社团。','学术',NULL,'college',1,107,1,'李老师',6,'college_only','approved',NULL,107,'2026-07-17 20:52:14','2026-07-18 16:27:11'),(6,'文学创作社','文学院学生的创作交流平台，定期举办征文比赛。','文艺',NULL,'college',2,103,1,'王老师',6,'college_only','approved',NULL,103,'2026-07-17 20:52:14','2026-07-18 16:27:11'),(7,'化学实验社','理学院化学爱好者社团，开展趣味化学实验。','学术',NULL,'college',3,104,1,'张老师',5,'college_only','approved',NULL,104,'2026-07-17 20:52:14','2026-07-18 16:27:11'),(8,'创业实践社','经济与管理学院创业实践平台，模拟创业项目。','实践',NULL,'college',5,105,1,'孙老师',5,'college_only','approved',NULL,105,'2026-07-17 20:52:14','2026-07-18 16:27:11'),(13,'PythonClub','Learning Python programming','tech',NULL,'college',1,NULL,1,'Prof. Wang',0,'all','pending',NULL,201,'2026-07-18 17:20:43','2026-07-18 17:20:43'),(14,'测试公告','测试内容','科技',NULL,'college',1,NULL,1,'王教授',0,'all','pending',NULL,201,'2026-07-18 17:20:54','2026-07-18 17:20:54'),(15,'PhotographyClub','Photography enthusiasts club','art',NULL,'college',1,201,1,'Prof. Li',1,'all','approved',NULL,201,'2026-07-18 17:23:09','2026-07-18 17:24:16');
/*!40000 ALTER TABLE `clubs` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `colleges`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `colleges` (
  `college_id` int NOT NULL AUTO_INCREMENT,
  `college_name` varchar(100) NOT NULL,
  `college_code` varchar(20) NOT NULL,
  `description` varchar(500) DEFAULT NULL,
  `status` tinyint NOT NULL DEFAULT '1',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`college_id`),
  UNIQUE KEY `college_name` (`college_name`),
  UNIQUE KEY `college_code` (`college_code`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `colleges` WRITE;
/*!40000 ALTER TABLE `colleges` DISABLE KEYS */;
INSERT INTO `colleges` VALUES (1,'信息科学与技术学院','CS','涵盖计算机、软件、大数据、人工智能等专业',1,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(2,'文学院','LIT','涵盖汉语言文学、新闻、广告等专业',1,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(3,'理学院','SCI','涵盖数学、物理、化学等基础学科',1,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(4,'工学院','ENG','涵盖土木、机械、电气等工程专业',1,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(5,'经济与管理学院','BIZ','涵盖经济学、工商管理、会计等专业',1,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(6,'医学院','MED','涵盖临床医学、护理、药学等专业',1,'2026-07-17 20:52:14','2026-07-17 20:52:14');
/*!40000 ALTER TABLE `colleges` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `election_candidates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `election_candidates` (
  `candidate_id` int NOT NULL AUTO_INCREMENT,
  `election_id` int NOT NULL,
  `user_id` int NOT NULL,
  `position` enum('president','vice_president') NOT NULL,
  `is_reelect` tinyint NOT NULL DEFAULT '0',
  `votes` int NOT NULL DEFAULT '0',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `signup_time` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`candidate_id`),
  UNIQUE KEY `uk_election_user_pos` (`election_id`,`user_id`,`position`),
  KEY `user_id` (`user_id`),
  KEY `idx_election` (`election_id`),
  CONSTRAINT `election_candidates_ibfk_1` FOREIGN KEY (`election_id`) REFERENCES `elections` (`election_id`) ON DELETE CASCADE,
  CONSTRAINT `election_candidates_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `election_candidates` WRITE;
/*!40000 ALTER TABLE `election_candidates` DISABLE KEYS */;
INSERT INTO `election_candidates` VALUES (1,1,101,'president',1,10,'2026-02-21 10:00:00','2026-07-18 16:28:22'),(2,1,205,'president',0,5,'2026-02-22 14:00:00','2026-07-18 16:28:22'),(3,1,202,'vice_president',1,12,'2026-02-21 11:00:00','2026-07-18 16:28:22'),(4,1,208,'vice_president',0,3,'2026-02-23 09:00:00','2026-07-18 16:28:22'),(5,2,203,'vice_president',0,2,'2026-07-16 10:00:00','2026-07-18 16:28:22'),(6,2,211,'vice_president',0,0,'2026-07-17 14:00:00','2026-07-18 16:28:22'),(7,2,214,'vice_president',0,0,'2026-07-18 09:00:00','2026-07-18 16:28:22'),(8,3,108,'president',1,5,'2026-07-02 10:00:00','2026-07-18 16:28:22'),(9,3,204,'president',0,3,'2026-07-03 14:00:00','2026-07-18 16:28:22'),(10,2,101,'president',1,0,'2026-07-18 16:52:36','2026-07-18 16:52:36');
/*!40000 ALTER TABLE `election_candidates` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `election_votes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `election_votes` (
  `vote_id` int NOT NULL AUTO_INCREMENT,
  `election_id` int NOT NULL,
  `candidate_id` int NOT NULL,
  `voter_id` int NOT NULL,
  `position` enum('president','vice_president') NOT NULL,
  `voted_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`vote_id`),
  UNIQUE KEY `uk_election_voter_pos` (`election_id`,`voter_id`,`position`),
  KEY `candidate_id` (`candidate_id`),
  KEY `voter_id` (`voter_id`),
  KEY `idx_election` (`election_id`),
  CONSTRAINT `election_votes_ibfk_1` FOREIGN KEY (`election_id`) REFERENCES `elections` (`election_id`) ON DELETE CASCADE,
  CONSTRAINT `election_votes_ibfk_2` FOREIGN KEY (`candidate_id`) REFERENCES `election_candidates` (`candidate_id`) ON DELETE CASCADE,
  CONSTRAINT `election_votes_ibfk_3` FOREIGN KEY (`voter_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `election_votes` WRITE;
/*!40000 ALTER TABLE `election_votes` DISABLE KEYS */;
/*!40000 ALTER TABLE `election_votes` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `elections`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `elections` (
  `election_id` int NOT NULL AUTO_INCREMENT,
  `club_id` int NOT NULL,
  `title` varchar(200) DEFAULT NULL,
  `initiated_by` int NOT NULL,
  `term_start` date NOT NULL,
  `term_end` date NOT NULL,
  `signup_start` datetime NOT NULL,
  `signup_end` datetime NOT NULL,
  `vote_start` datetime NOT NULL,
  `vote_end` datetime NOT NULL,
  `president_count` int NOT NULL DEFAULT '1',
  `vice_president_count` int NOT NULL DEFAULT '1',
  `status` enum('signup','voting','completed','cancelled','no_candidate') NOT NULL DEFAULT 'signup',
  `result_summary` text,
  `settled_at` datetime DEFAULT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`election_id`),
  KEY `initiated_by` (`initiated_by`),
  KEY `idx_club` (`club_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `elections_ibfk_1` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE,
  CONSTRAINT `elections_ibfk_2` FOREIGN KEY (`initiated_by`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `elections` WRITE;
/*!40000 ALTER TABLE `elections` DISABLE KEYS */;
INSERT INTO `elections` VALUES (1,1,'ACM编程协会2026年换届选举',101,'2026-03-01','2027-03-01','2026-02-20 00:00:00','2026-03-01 23:59:59','2026-03-02 00:00:00','2026-03-10 23:59:59',1,1,'completed',NULL,NULL,'2026-02-15 10:00:00'),(2,1,'ACM编程协会副社长补选',101,'2026-07-15','2027-07-15','2026-07-15 00:00:00','2026-07-30 23:59:59','2026-08-01 00:00:00','2026-07-15 23:59:59',0,1,'completed',NULL,'2026-07-18 16:53:07','2026-07-10 10:00:00'),(3,4,'绿茵足球社2026年换届选举',108,'2026-07-10','2027-07-10','2026-07-01 00:00:00','2026-07-10 23:59:59','2026-07-11 00:00:00','2026-07-25 23:59:59',1,0,'voting',NULL,NULL,'2026-06-20 14:00:00'),(4,6,'文学创作社2026年换届选举',103,'2026-03-05','2027-03-05','2026-02-20 00:00:00','2026-03-05 23:59:59','2026-03-06 00:00:00','2026-03-15 23:59:59',1,1,'completed',NULL,NULL,'2026-02-18 09:00:00');
/*!40000 ALTER TABLE `elections` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `finance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `finance` (
  `finance_id` int NOT NULL AUTO_INCREMENT,
  `club_id` int NOT NULL,
  `type` enum('income','expense') NOT NULL,
  `amount` decimal(12,2) NOT NULL,
  `source` varchar(50) DEFAULT NULL,
  `description` varchar(500) DEFAULT NULL,
  `operator_id` int DEFAULT NULL,
  `record_time` datetime DEFAULT CURRENT_TIMESTAMP,
  `receipt_path` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`finance_id`),
  KEY `operator_id` (`operator_id`),
  KEY `idx_club` (`club_id`),
  KEY `idx_type` (`type`),
  KEY `idx_time` (`record_time`),
  CONSTRAINT `finance_ibfk_1` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE,
  CONSTRAINT `finance_ibfk_2` FOREIGN KEY (`operator_id`) REFERENCES `users` (`user_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `finance` WRITE;
/*!40000 ALTER TABLE `finance` DISABLE KEYS */;
INSERT INTO `finance` VALUES (1,1,'income',3000.00,NULL,'学校社团经费拨款（2026春季学期）',101,'2026-07-18 16:28:22',NULL),(2,1,'expense',450.00,NULL,'ACM新生赛奖品采购（图书+证书）',101,'2026-07-18 16:28:22',NULL),(3,1,'expense',180.00,NULL,'训练赛零食补给',202,'2026-07-18 16:28:22',NULL),(4,1,'income',500.00,NULL,'会员会费（2026级新生）',NULL,'2026-07-18 16:28:22',NULL),(5,2,'income',2000.00,NULL,'学校社团经费拨款',102,'2026-07-18 16:28:22',NULL),(6,2,'expense',320.00,NULL,'敬老院慰问物资采购（水果+牛奶）',102,'2026-07-18 16:28:22',NULL),(7,2,'expense',150.00,NULL,'环保活动工具采购',NULL,'2026-07-18 16:28:22',NULL),(8,3,'income',1500.00,NULL,'学校社团经费拨款',106,'2026-07-18 16:28:22',NULL),(9,3,'expense',400.00,NULL,'观测之夜交通费（包车往返观测点）',106,'2026-07-18 16:28:22',NULL),(10,3,'expense',200.00,NULL,'望远镜配件购买',NULL,'2026-07-18 16:28:22',NULL),(11,4,'income',2500.00,NULL,'学校社团经费拨款',108,'2026-07-18 16:28:22',NULL),(12,4,'expense',600.00,NULL,'足球采购（比赛用球x3）',108,'2026-07-18 16:28:22',NULL),(13,4,'expense',350.00,NULL,'队服印号费用',NULL,'2026-07-18 16:28:22',NULL),(14,5,'income',1200.00,NULL,'学院社团经费拨款',107,'2026-07-18 16:28:22',NULL),(15,5,'expense',280.00,NULL,'算法书籍采购',NULL,'2026-07-18 16:28:22',NULL),(16,6,'income',1000.00,NULL,'学院社团经费拨款',103,'2026-07-18 16:28:22',NULL),(17,6,'expense',150.00,NULL,'读书会茶歇费用',NULL,'2026-07-18 16:28:22',NULL),(18,7,'income',1500.00,NULL,'学院社团经费拨款',104,'2026-07-18 16:28:22',NULL),(19,7,'expense',380.00,NULL,'实验耗材采购（试剂+模具）',NULL,'2026-07-18 16:28:22',NULL),(20,8,'income',1200.00,NULL,'学院社团经费拨款',105,'2026-07-18 16:28:22',NULL),(21,8,'expense',220.00,NULL,'创业计划大赛宣传物料',NULL,'2026-07-18 16:28:22',NULL);
/*!40000 ALTER TABLE `finance` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `logs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `logs` (
  `log_id` int NOT NULL AUTO_INCREMENT,
  `user_id` int DEFAULT NULL,
  `action` varchar(100) NOT NULL,
  `target_type` varchar(50) DEFAULT NULL,
  `target_id` int DEFAULT NULL,
  `detail` text,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`),
  KEY `idx_user` (`user_id`),
  KEY `idx_action` (`action`),
  KEY `idx_time` (`created_at`),
  CONSTRAINT `logs_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=313 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `logs` WRITE;
/*!40000 ALTER TABLE `logs` DISABLE KEYS */;
INSERT INTO `logs` VALUES (1,101,'login','system',NULL,'用户登录系统','2026-07-18 16:28:22'),(2,1,'approve','clubs',1,'通过ACM编程协会创建申请','2026-07-18 16:28:22'),(3,1,'approve','clubs',2,'通过志愿者协会创建申请','2026-07-18 16:28:22'),(4,2,'approve','reimbursement',1,'审批通过信科算法社书籍采购报销','2026-07-18 16:28:22'),(5,3,'approve','reimbursement',2,'审批通过文学创作社茶歇报销','2026-07-18 16:28:22'),(6,3,'reject','reimbursement',8,'驳回志愿者协会志愿者马甲定制报销','2026-07-18 16:28:22'),(7,101,'create','activities',1,'创建活动\"ACM新生算法入门赛\"','2026-07-18 16:28:22'),(8,108,'create','activities',9,'创建活动\"院际足球联赛第一轮\"','2026-07-18 16:28:22'),(9,106,'create','elections',2,'发起\"ACM编程协会副社长补选\"选举','2026-07-18 16:28:22'),(10,1,'post','announcements',NULL,'发布学校公告\"2026年秋季社团招新工作安排\"','2026-07-18 16:28:22'),(11,1,'login','users',1,'用户登录','2026-07-18 16:31:46'),(12,1,'login','users',1,'用户登录','2026-07-18 16:31:46'),(13,1,'login','users',1,'用户登录','2026-07-18 16:44:51'),(14,1,'login','users',1,'用户登录','2026-07-18 16:44:51'),(15,1,'login','users',1,'用户登录','2026-07-18 16:44:54'),(16,1,'login','users',1,'用户登录','2026-07-18 16:44:54'),(17,1,'login','users',1,'用户登录','2026-07-18 16:45:01'),(18,1,'login','users',1,'用户登录','2026-07-18 16:45:01'),(19,1,'login','users',1,'用户登录','2026-07-18 16:45:01'),(20,1,'login','users',1,'用户登录','2026-07-18 16:45:01'),(21,1,'login','users',1,'用户登录','2026-07-18 16:45:05'),(22,1,'login','users',1,'用户登录','2026-07-18 16:45:05'),(23,1,'db_backup','backup_records',1,'手动数据库备份','2026-07-18 16:45:21'),(24,2,'login','users',2,'用户登录','2026-07-18 16:45:58'),(25,2,'login','users',2,'用户登录','2026-07-18 16:45:58'),(26,2,'login','users',2,'用户登录','2026-07-18 16:46:03'),(27,2,'login','users',2,'用户登录','2026-07-18 16:46:03'),(28,201,'login','users',201,'用户登录','2026-07-18 16:46:30'),(29,201,'login','users',201,'用户登录','2026-07-18 16:46:30'),(30,101,'login','users',101,'用户登录','2026-07-18 16:46:53'),(31,101,'login','users',101,'用户登录','2026-07-18 16:46:53'),(32,101,'login','users',101,'用户登录','2026-07-18 16:49:34'),(33,101,'login','users',101,'用户登录','2026-07-18 16:49:34'),(34,202,'login','users',202,'用户登录','2026-07-18 16:49:54'),(35,202,'login','users',202,'用户登录','2026-07-18 16:49:54'),(36,203,'login','users',203,'用户登录','2026-07-18 16:51:38'),(37,203,'login','users',203,'用户登录','2026-07-18 16:51:38'),(38,101,'login','users',101,'用户登录','2026-07-18 16:52:07'),(39,101,'login','users',101,'用户登录','2026-07-18 16:52:07'),(40,1,'login','users',1,'用户登录','2026-07-18 16:53:25'),(41,1,'login','users',1,'用户登录','2026-07-18 16:53:25'),(42,2,'login','users',2,'用户登录','2026-07-18 16:53:25'),(43,2,'login','users',2,'用户登录','2026-07-18 16:53:25'),(44,1,'delete_announcement','announcements',1,'删除公告','2026-07-18 16:53:25'),(45,2,'post_announcement','announcements',44,'发布学院公告','2026-07-18 16:53:26'),(46,1,'login','users',1,'用户登录','2026-07-18 16:55:13'),(47,1,'login','users',1,'用户登录','2026-07-18 16:55:13'),(48,202,'login','users',202,'用户登录','2026-07-18 16:55:13'),(49,202,'login','users',202,'用户登录','2026-07-18 16:55:13'),(50,203,'login','users',203,'用户登录','2026-07-18 16:55:13'),(51,203,'login','users',203,'用户登录','2026-07-18 16:55:13'),(52,1,'db_backup','backup_records',2,'手动数据库备份','2026-07-18 16:55:14'),(53,202,'login','users',202,'用户登录','2026-07-18 16:55:19'),(54,202,'login','users',202,'用户登录','2026-07-18 16:55:19'),(55,202,'login','users',202,'用户登录','2026-07-18 16:55:29'),(56,202,'login','users',202,'用户登录','2026-07-18 16:55:29'),(57,1,'login','users',1,'用户登录','2026-07-18 17:12:13'),(58,1,'login','users',1,'用户登录','2026-07-18 17:12:13'),(59,1,'login','users',1,'用户登录','2026-07-18 17:12:18'),(60,1,'login','users',1,'用户登录','2026-07-18 17:12:18'),(61,1,'login','users',1,'用户登录','2026-07-18 17:12:22'),(62,1,'login','users',1,'用户登录','2026-07-18 17:12:22'),(63,1,'post_announcement','announcements',8,'发布全校公告','2026-07-18 17:12:31'),(64,102,'login','users',102,'用户登录','2026-07-18 17:12:38'),(65,102,'login','users',102,'用户登录','2026-07-18 17:12:38'),(66,1,'post_announcement','announcements',9,'发布全校公告','2026-07-18 17:12:58'),(67,1,'post_announcement','announcements',10,'发布全校公告','2026-07-18 17:13:02'),(68,1,'post_announcement','announcements',11,'发布全校公告','2026-07-18 17:13:02'),(69,1,'post_announcement','announcements',12,'发布全校公告','2026-07-18 17:17:43'),(70,1,'post_announcement','announcements',13,'发布全校公告','2026-07-18 17:17:51'),(71,1,'post_announcement','announcements',14,'发布全校公告','2026-07-18 17:18:04'),(72,1,'post_announcement','announcements',15,'发布全校公告','2026-07-18 17:18:05'),(73,1,'post_announcement','announcements',16,'发布全校公告','2026-07-18 17:18:05'),(74,1,'post_announcement','announcements',17,'发布全校公告','2026-07-18 17:18:05'),(75,1,'post_announcement','announcements',18,'发布全校公告','2026-07-18 17:18:06'),(76,1,'post_announcement','announcements',19,'发布全校公告','2026-07-18 17:18:07'),(77,1,'post_announcement','announcements',20,'发布全校公告','2026-07-18 17:18:08'),(78,1,'post_announcement','announcements',21,'发布全校公告','2026-07-18 17:18:08'),(79,1,'post_announcement','announcements',22,'发布全校公告','2026-07-18 17:18:09'),(80,1,'post_announcement','announcements',23,'发布全校公告','2026-07-18 17:18:10'),(81,1,'post_announcement','announcements',24,'发布全校公告','2026-07-18 17:18:31'),(82,1,'post_announcement','announcements',25,'发布全校公告','2026-07-18 17:18:31'),(83,1,'post_announcement','announcements',26,'发布全校公告','2026-07-18 17:18:31'),(84,1,'post_announcement','announcements',27,'发布全校公告','2026-07-18 17:18:31'),(85,1,'post_announcement','announcements',28,'发布全校公告','2026-07-18 17:18:32'),(86,1,'post_announcement','announcements',29,'发布全校公告','2026-07-18 17:18:49'),(87,201,'login','users',201,'用户登录','2026-07-18 17:20:39'),(88,201,'login','users',201,'用户登录','2026-07-18 17:20:39'),(89,101,'login','users',101,'用户登录','2026-07-18 17:21:08'),(90,101,'login','users',101,'用户登录','2026-07-18 17:21:08'),(91,1,'login','users',1,'用户登录','2026-07-18 17:23:09'),(92,1,'login','users',1,'用户登录','2026-07-18 17:23:09'),(93,1,'post_announcement','announcements',30,'发布全校公告','2026-07-18 17:23:09'),(94,1,'approve_club','clubs',15,'审批通过社团','2026-07-18 17:24:16'),(95,2,'login','users',2,'用户登录','2026-07-18 17:24:50'),(96,2,'login','users',2,'用户登录','2026-07-18 17:24:50'),(97,2,'post_announcement','announcements',31,'发布学院公告','2026-07-18 17:24:50'),(98,201,'login','users',201,'用户登录','2026-07-18 17:25:01'),(99,201,'login','users',201,'用户登录','2026-07-18 17:25:01'),(100,203,'login','users',203,'用户登录','2026-07-18 17:25:12'),(101,203,'login','users',203,'用户登录','2026-07-18 17:25:12'),(102,101,'login','users',101,'用户登录','2026-07-18 17:27:18'),(103,101,'login','users',101,'用户登录','2026-07-18 17:27:18'),(104,203,'login','users',203,'用户登录','2026-07-18 17:27:18'),(105,203,'login','users',203,'用户登录','2026-07-18 17:27:18'),(106,1,'login','users',1,'用户登录','2026-07-18 17:28:33'),(107,1,'login','users',1,'用户登录','2026-07-18 17:28:33'),(108,1,'db_backup','backup_records',3,'手动数据库备份','2026-07-18 17:28:33'),(109,278,'login','users',278,'用户登录','2026-07-18 17:29:21'),(110,278,'login','users',278,'用户登录','2026-07-18 17:29:21'),(111,1,'set_config','system_config',NULL,'修改系统配置','2026-07-18 17:30:00'),(112,1,'reset_password','users',278,'重置用户密码','2026-07-18 17:30:00'),(113,1,'login','users',1,'用户登录','2026-07-18 17:42:28'),(114,1,'login','users',1,'用户登录','2026-07-18 17:42:28'),(115,1,'login','users',1,'用户登录','2026-07-18 17:52:25'),(116,1,'login','users',1,'用户登录','2026-07-18 17:52:25'),(117,1,'login','users',1,'用户登录','2026-07-18 17:55:19'),(118,1,'login','users',1,'用户登录','2026-07-18 17:55:19'),(119,279,'login','users',279,'用户登录','2026-07-18 18:07:24'),(120,279,'login','users',279,'用户登录','2026-07-18 18:07:24'),(121,285,'login','users',285,'用户登录','2026-07-18 18:07:24'),(122,285,'login','users',285,'用户登录','2026-07-18 18:07:24'),(123,279,'login','users',279,'用户登录','2026-07-18 18:09:14'),(124,279,'login','users',279,'用户登录','2026-07-18 18:09:14'),(125,285,'login','users',285,'用户登录','2026-07-18 18:10:16'),(126,285,'login','users',285,'用户登录','2026-07-18 18:10:16'),(127,285,'login','users',285,'用户登录','2026-07-18 18:10:18'),(128,285,'login','users',285,'用户登录','2026-07-18 18:10:18'),(129,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(130,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(131,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(132,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(133,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(134,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(135,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(136,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(137,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(138,285,'login','users',285,'用户登录','2026-07-18 18:10:19'),(139,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(140,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(141,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(142,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(143,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(144,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(145,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(146,285,'login','users',285,'用户登录','2026-07-18 18:10:20'),(147,285,'login','users',285,'用户登录','2026-07-18 18:11:50'),(148,285,'login','users',285,'用户登录','2026-07-18 18:11:50'),(149,285,'login','users',285,'用户登录','2026-07-18 18:11:52'),(150,285,'login','users',285,'用户登录','2026-07-18 18:11:52'),(151,285,'login','users',285,'用户登录','2026-07-18 18:11:52'),(152,285,'login','users',285,'用户登录','2026-07-18 18:11:52'),(153,285,'login','users',285,'用户登录','2026-07-18 18:11:53'),(154,285,'login','users',285,'用户登录','2026-07-18 18:11:53'),(155,285,'login','users',285,'用户登录','2026-07-18 18:13:19'),(156,285,'login','users',285,'用户登录','2026-07-18 18:13:19'),(157,285,'login','users',285,'用户登录','2026-07-18 18:17:09'),(158,285,'login','users',285,'用户登录','2026-07-18 18:17:09'),(159,1,'login','users',1,'用户登录','2026-07-18 18:49:21'),(160,1,'login','users',1,'用户登录','2026-07-18 18:49:21'),(161,101,'login','users',101,'用户登录','2026-07-18 18:49:24'),(162,101,'login','users',101,'用户登录','2026-07-18 18:49:24'),(163,1,'login','users',1,'用户登录','2026-07-18 18:49:34'),(164,1,'login','users',1,'用户登录','2026-07-18 18:49:34'),(165,101,'login','users',101,'用户登录','2026-07-18 18:49:37'),(166,101,'login','users',101,'用户登录','2026-07-18 18:49:37'),(167,101,'login','users',101,'用户登录','2026-07-18 18:49:39'),(168,101,'login','users',101,'用户登录','2026-07-18 18:49:39'),(169,1,'login','users',1,'用户登录','2026-07-18 18:49:41'),(170,1,'login','users',1,'用户登录','2026-07-18 18:49:41'),(171,1,'login','users',1,'用户登录','2026-07-18 18:49:49'),(172,1,'login','users',1,'用户登录','2026-07-18 18:49:49'),(173,101,'login','users',101,'用户登录','2026-07-18 18:49:50'),(174,101,'login','users',101,'用户登录','2026-07-18 18:49:50'),(175,101,'login','users',101,'用户登录','2026-07-18 19:04:56'),(176,101,'login','users',101,'用户登录','2026-07-18 19:04:56'),(177,1,'login','users',1,'用户登录','2026-07-18 19:06:57'),(178,1,'login','users',1,'用户登录','2026-07-18 19:06:57'),(179,101,'login','users',101,'用户登录','2026-07-18 19:06:58'),(180,101,'login','users',101,'用户登录','2026-07-18 19:06:58'),(181,285,'login','users',285,'用户登录','2026-07-18 19:11:23'),(182,285,'login','users',285,'用户登录','2026-07-18 19:11:23'),(183,1,'login','users',1,'用户登录','2026-07-18 19:12:58'),(184,1,'login','users',1,'用户登录','2026-07-18 19:12:58'),(185,279,'login','users',279,'用户登录','2026-07-18 19:18:34'),(186,279,'login','users',279,'用户登录','2026-07-18 19:18:34'),(187,285,'login','users',285,'用户登录','2026-07-18 19:21:10'),(188,285,'login','users',285,'用户登录','2026-07-18 19:21:10'),(189,2,'login','users',2,'用户登录','2026-07-18 19:21:49'),(190,2,'login','users',2,'用户登录','2026-07-18 19:21:49'),(191,2,'login','users',2,'用户登录','2026-07-18 19:32:03'),(192,2,'login','users',2,'用户登录','2026-07-18 19:32:03'),(193,285,'login','users',285,'用户登录','2026-07-18 19:39:38'),(194,285,'login','users',285,'用户登录','2026-07-18 19:39:38'),(195,2,'login','users',2,'用户登录','2026-07-18 19:39:43'),(196,2,'login','users',2,'用户登录','2026-07-18 19:39:43'),(197,101,'login','users',101,'用户登录','2026-07-18 19:39:48'),(198,101,'login','users',101,'用户登录','2026-07-18 19:39:48'),(199,1,'login','users',1,'用户登录','2026-07-18 19:40:01'),(200,1,'login','users',1,'用户登录','2026-07-18 19:40:01'),(201,287,'login','users',287,'用户登录','2026-07-18 19:56:35'),(202,287,'login','users',287,'用户登录','2026-07-18 19:56:35'),(203,1,'login','users',1,'用户登录','2026-07-18 20:25:55'),(204,1,'login','users',1,'用户登录','2026-07-18 20:25:55'),(205,1,'login','users',1,'用户登录','2026-07-18 20:27:29'),(206,1,'login','users',1,'用户登录','2026-07-18 20:27:29'),(207,1,'login','users',1,'用户登录','2026-07-18 20:28:40'),(208,1,'login','users',1,'用户登录','2026-07-18 20:28:40'),(209,287,'login','users',287,'用户登录','2026-07-18 20:30:12'),(210,287,'login','users',287,'用户登录','2026-07-18 20:30:12'),(211,1,'login','users',1,'用户登录','2026-07-18 20:41:26'),(212,1,'login','users',1,'用户登录','2026-07-18 20:41:26'),(213,201,'login','users',201,'用户登录','2026-07-18 20:44:08'),(214,201,'login','users',201,'用户登录','2026-07-18 20:44:08'),(215,201,'login','users',201,'用户登录','2026-07-18 20:44:16'),(216,201,'login','users',201,'用户登录','2026-07-18 20:44:16'),(217,201,'login','users',201,'用户登录','2026-07-18 20:45:04'),(218,201,'login','users',201,'用户登录','2026-07-18 20:45:04'),(219,1,'login','users',1,'用户登录','2026-07-18 20:50:15'),(220,1,'login','users',1,'用户登录','2026-07-18 20:50:15'),(221,101,'login','users',101,'用户登录','2026-07-18 20:51:39'),(222,101,'login','users',101,'用户登录','2026-07-18 20:51:39'),(223,201,'login','users',201,'用户登录','2026-07-18 20:51:49'),(224,201,'login','users',201,'用户登录','2026-07-18 20:51:49'),(225,201,'login','users',201,'用户登录','2026-07-18 20:53:25'),(226,201,'login','users',201,'用户登录','2026-07-18 20:53:25'),(227,201,'login','users',201,'用户登录','2026-07-18 20:54:14'),(228,201,'login','users',201,'用户登录','2026-07-18 20:54:14'),(229,201,'login','users',201,'用户登录','2026-07-18 20:55:12'),(230,201,'login','users',201,'用户登录','2026-07-18 20:55:12'),(231,201,'login','users',201,'用户登录','2026-07-18 20:55:49'),(232,201,'login','users',201,'用户登录','2026-07-18 20:55:49'),(233,201,'login','users',201,'用户登录','2026-07-18 20:55:56'),(234,201,'login','users',201,'用户登录','2026-07-18 20:55:56'),(235,201,'login','users',201,'用户登录','2026-07-18 20:56:03'),(236,201,'login','users',201,'用户登录','2026-07-18 20:56:03'),(237,287,'login','users',287,'用户登录','2026-07-18 21:06:59'),(238,287,'login','users',287,'用户登录','2026-07-18 21:06:59'),(239,101,'login','users',101,'用户登录','2026-07-18 21:09:03'),(240,101,'login','users',101,'用户登录','2026-07-18 21:09:03'),(241,101,'login','users',101,'用户登录','2026-07-18 21:19:05'),(242,101,'login','users',101,'用户登录','2026-07-18 21:19:05'),(243,101,'login','users',101,'用户登录','2026-07-18 21:19:26'),(244,101,'login','users',101,'用户登录','2026-07-18 21:19:26'),(245,101,'login','users',101,'用户登录','2026-07-18 21:21:17'),(246,101,'login','users',101,'用户登录','2026-07-18 21:21:17'),(247,101,'login','users',101,'用户登录','2026-07-18 21:24:36'),(248,101,'login','users',101,'用户登录','2026-07-18 21:24:36'),(249,101,'login','users',101,'用户登录','2026-07-18 21:27:23'),(250,101,'login','users',101,'用户登录','2026-07-18 21:27:23'),(251,101,'login','users',101,'用户登录','2026-07-18 21:33:26'),(252,101,'login','users',101,'用户登录','2026-07-18 21:33:26'),(253,101,'login','users',101,'用户登录','2026-07-18 21:40:00'),(254,101,'login','users',101,'用户登录','2026-07-18 21:40:00'),(255,101,'login','users',101,'用户登录','2026-07-18 21:45:48'),(256,101,'login','users',101,'用户登录','2026-07-18 21:45:48'),(257,101,'login','users',101,'用户登录','2026-07-18 21:45:56'),(258,101,'login','users',101,'用户登录','2026-07-18 21:45:56'),(259,101,'login','users',101,'用户登录','2026-07-18 21:46:07'),(260,101,'login','users',101,'用户登录','2026-07-18 21:46:07'),(261,2,'login','users',2,'用户登录','2026-07-18 21:46:07'),(262,2,'login','users',2,'用户登录','2026-07-18 21:46:07'),(263,101,'login','users',101,'用户登录','2026-07-18 21:48:17'),(264,101,'login','users',101,'用户登录','2026-07-18 21:48:17'),(265,101,'login','users',101,'用户登录','2026-07-18 22:46:52'),(266,101,'login','users',101,'用户登录','2026-07-18 22:46:52'),(267,101,'login','users',101,'用户登录','2026-07-18 22:47:01'),(268,101,'login','users',101,'用户登录','2026-07-18 22:47:01'),(269,101,'login','users',101,'用户登录','2026-07-18 22:47:09'),(270,101,'login','users',101,'用户登录','2026-07-18 22:47:09'),(271,101,'login','users',101,'用户登录','2026-07-18 22:47:25'),(272,101,'login','users',101,'用户登录','2026-07-18 22:47:25'),(273,101,'login','users',101,'用户登录','2026-07-18 22:47:44'),(274,101,'login','users',101,'用户登录','2026-07-18 22:47:44'),(275,101,'login','users',101,'用户登录','2026-07-18 22:58:11'),(276,101,'login','users',101,'用户登录','2026-07-18 22:58:11'),(277,101,'login','users',101,'用户登录','2026-07-18 23:13:13'),(278,101,'login','users',101,'用户登录','2026-07-18 23:13:13'),(279,101,'login','users',101,'用户登录','2026-07-18 23:13:29'),(280,101,'login','users',101,'用户登录','2026-07-18 23:13:29'),(281,101,'login','users',101,'用户登录','2026-07-18 23:15:06'),(282,101,'login','users',101,'用户登录','2026-07-18 23:15:06'),(283,101,'login','users',101,'用户登录','2026-07-18 23:16:53'),(284,101,'login','users',101,'用户登录','2026-07-18 23:16:53'),(285,101,'login','users',101,'用户登录','2026-07-18 23:23:38'),(286,101,'login','users',101,'用户登录','2026-07-18 23:23:38'),(287,101,'login','users',101,'用户登录','2026-07-18 23:28:58'),(288,101,'login','users',101,'用户登录','2026-07-18 23:28:58'),(289,101,'login','users',101,'用户登录','2026-07-18 23:35:50'),(290,101,'login','users',101,'用户登录','2026-07-18 23:35:50'),(291,101,'login','users',101,'用户登录','2026-07-19 10:14:29'),(292,101,'login','users',101,'用户登录','2026-07-19 10:14:29'),(293,1,'login','users',1,'用户登录','2026-07-19 10:19:13'),(294,1,'login','users',1,'用户登录','2026-07-19 10:19:13'),(295,101,'login','users',101,'用户登录','2026-07-19 10:19:34'),(296,101,'login','users',101,'用户登录','2026-07-19 10:19:34'),(297,1,'login','users',1,'用户登录','2026-07-19 10:36:44'),(298,1,'login','users',1,'用户登录','2026-07-19 10:36:44'),(299,1,'login','users',1,'用户登录','2026-07-19 10:37:17'),(300,1,'login','users',1,'用户登录','2026-07-19 10:37:17'),(301,101,'login','users',101,'用户登录','2026-07-19 10:42:57'),(302,101,'login','users',101,'用户登录','2026-07-19 10:42:57'),(303,1,'login','users',1,'用户登录','2026-07-19 11:02:32'),(304,1,'login','users',1,'用户登录','2026-07-19 11:02:32'),(305,1,'login','users',1,'用户登录','2026-07-19 11:17:52'),(306,1,'login','users',1,'用户登录','2026-07-19 11:17:52'),(307,1,'login','users',1,'用户登录','2026-07-19 11:25:04'),(308,1,'login','users',1,'用户登录','2026-07-19 11:25:04'),(309,1,'login','users',1,'用户登录','2026-07-19 11:27:17'),(310,1,'login','users',1,'用户登录','2026-07-19 11:27:17'),(311,1,'login','users',1,'用户登录','2026-07-19 11:30:12'),(312,1,'login','users',1,'用户登录','2026-07-19 11:30:12');
/*!40000 ALTER TABLE `logs` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `majors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `majors` (
  `major_id` int NOT NULL AUTO_INCREMENT,
  `college_id` int NOT NULL,
  `major_name` varchar(100) NOT NULL,
  `major_code` varchar(20) DEFAULT NULL,
  `status` tinyint NOT NULL DEFAULT '1',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`major_id`),
  UNIQUE KEY `uk_college_major` (`college_id`,`major_name`),
  CONSTRAINT `majors_ibfk_1` FOREIGN KEY (`college_id`) REFERENCES `colleges` (`college_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=20 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `majors` WRITE;
/*!40000 ALTER TABLE `majors` DISABLE KEYS */;
INSERT INTO `majors` VALUES (1,1,'计算机科学与技术','CS-01',1,'2026-07-17 20:52:14'),(2,1,'软件工程','CS-02',1,'2026-07-17 20:52:14'),(3,1,'数据科学与大数据技术','CS-03',1,'2026-07-17 20:52:14'),(4,1,'人工智能','CS-04',1,'2026-07-17 20:52:14'),(5,2,'汉语言文学','LIT-01',1,'2026-07-17 20:52:14'),(6,2,'新闻学','LIT-02',1,'2026-07-17 20:52:14'),(7,2,'广告学','LIT-03',1,'2026-07-17 20:52:14'),(8,3,'数学与应用数学','SCI-01',1,'2026-07-17 20:52:14'),(9,3,'物理学','SCI-02',1,'2026-07-17 20:52:14'),(10,3,'化学','SCI-03',1,'2026-07-17 20:52:14'),(11,4,'土木工程','ENG-01',1,'2026-07-17 20:52:14'),(12,4,'机械工程','ENG-02',1,'2026-07-17 20:52:14'),(13,4,'电气工程','ENG-03',1,'2026-07-17 20:52:14'),(14,5,'经济学','BIZ-01',1,'2026-07-17 20:52:14'),(15,5,'工商管理','BIZ-02',1,'2026-07-17 20:52:14'),(16,5,'会计学','BIZ-03',1,'2026-07-17 20:52:14'),(17,6,'临床医学','MED-01',1,'2026-07-17 20:52:14'),(18,6,'护理学','MED-02',1,'2026-07-17 20:52:14'),(19,6,'药学','MED-03',1,'2026-07-17 20:52:14');
/*!40000 ALTER TABLE `majors` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `members`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `members` (
  `member_id` int NOT NULL AUTO_INCREMENT,
  `club_id` int NOT NULL,
  `user_id` int NOT NULL,
  `role` enum('president','vice_president','secretary','treasurer','member') NOT NULL DEFAULT 'member',
  `join_status` enum('pending','approved','rejected') NOT NULL DEFAULT 'pending',
  `joined_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `left_at` datetime DEFAULT NULL,
  `attachment` varchar(255) DEFAULT NULL COMMENT 'uploaded application file',
  PRIMARY KEY (`member_id`),
  UNIQUE KEY `uk_club_user` (`club_id`,`user_id`),
  KEY `idx_user` (`user_id`),
  KEY `idx_club` (`club_id`),
  CONSTRAINT `members_ibfk_1` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE,
  CONSTRAINT `members_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1034 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `members` WRITE;
/*!40000 ALTER TABLE `members` DISABLE KEYS */;
INSERT INTO `members` VALUES (1015,1,202,'vice_president','approved','2026-07-18 16:31:26',NULL,NULL),(1016,1,203,'vice_president','approved','2026-07-18 16:31:26','2026-07-18 17:28:33',NULL),(1017,1,205,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1018,1,208,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1019,1,211,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1020,1,214,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1021,1,216,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1022,1,219,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1023,2,206,'vice_president','approved','2026-07-18 16:31:26',NULL,NULL),(1024,2,209,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1025,2,212,'member','approved','2026-07-18 16:31:26',NULL,NULL),(1026,1,201,'member','approved','2026-07-18 16:46:45','2026-07-18 16:51:56',NULL),(1027,4,203,'member','pending','2026-07-18 16:51:46',NULL,NULL),(1028,1,101,'president','approved','2026-07-18 16:52:23',NULL,NULL),(1030,15,201,'president','approved','2026-07-18 17:24:16',NULL,NULL),(1031,1,278,'member','approved','2026-07-18 17:29:21',NULL,NULL),(1032,1,287,'member','approved','2026-07-18 21:07:12',NULL,'storage/receipts/1784380032_350224.docx'),(1033,2,101,'member','pending','2026-07-18 22:18:38',NULL,NULL);
/*!40000 ALTER TABLE `members` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_member_after_insert` AFTER INSERT ON `members` FOR EACH ROW BEGIN
    IF NEW.join_status = 'approved' AND NEW.left_at IS NULL THEN
        UPDATE clubs SET member_count = member_count + 1 WHERE club_id = NEW.club_id;
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_member_after_update` AFTER UPDATE ON `members` FOR EACH ROW BEGIN
    
    IF NEW.join_status = 'approved' AND OLD.join_status <> 'approved'
       AND NEW.left_at IS NULL THEN
        UPDATE clubs SET member_count = member_count + 1 WHERE club_id = NEW.club_id;
    END IF;
    
    IF NEW.left_at IS NOT NULL AND OLD.left_at IS NULL
       AND OLD.join_status = 'approved' THEN
        UPDATE clubs SET member_count = member_count - 1 WHERE club_id = NEW.club_id;
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
DROP TABLE IF EXISTS `notifications`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `notifications` (
  `notification_id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `title` varchar(200) COLLATE utf8mb4_unicode_ci NOT NULL,
  `content` text COLLATE utf8mb4_unicode_ci NOT NULL,
  `type` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `related_id` int DEFAULT NULL,
  `is_read` tinyint NOT NULL DEFAULT '0',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`notification_id`),
  KEY `idx_user_read` (`user_id`,`is_read`),
  KEY `idx_time` (`created_at`),
  CONSTRAINT `notifications_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1553 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `notifications` WRITE;
/*!40000 ALTER TABLE `notifications` DISABLE KEYS */;
INSERT INTO `notifications` VALUES (1534,101,'系统通知：学期注册开放','新学期网上注册已开放，请登录教务系统完成学期注册','system',NULL,1,'2026-07-10 09:00:00'),(1535,101,'系统通知：系统维护公告','SCMS系统将于7月20日凌晨2:00-6:00进行升级维护，届时暂停服务','system',NULL,0,'2026-07-15 14:00:00'),(1536,101,'入社申请已通过','恭喜！你已被批准加入ACM编程协会，现为社团正式成员','join_result',1,1,'2026-07-08 16:30:00'),(1537,101,'ACM编程协会换届选举开始','社团换届选举投票阶段已开启，请前往投票页面参与投票','club',1,0,'2026-07-14 12:00:00'),(1538,101,'你在选举中被任命为新社长','经选举投票，你已被任命为ACM编程协会新任社长','role_change',1,1,'2026-07-16 10:00:00'),(1539,101,'社团公告：新学期活动计划','ACM编程协会新学期将组织蓝桥杯集训和ACM区域赛选拔','club_announce',1,0,'2026-07-17 19:00:00'),(1540,101,'新成员加入社团','张伟杰已通过入社审批，加入ACM编程协会','club',1,1,'2026-07-12 15:20:00'),(1541,101,'活动报名成功：2026迎新算法赛','你已成功报名2026迎新算法赛，比赛时间为9月15日14:00','activity',6,1,'2026-07-13 11:00:00'),(1542,101,'活动签到提醒：Demo Activity','你报名的Demo Activity即将在今天下午开始，请准时参加','activity',7,0,'2026-07-17 08:00:00'),(1543,101,'活动取消通知','原定于8月20日的暑期讲座活动因场地原因取消，敬请谅解','activity_cancelled',4,0,'2026-07-16 17:00:00'),(1544,202,'系统通知：学期注册开放','新学期网上注册已开放，请登录教务系统完成学期注册','system',NULL,1,'2026-07-10 09:00:00'),(1552,102,'新的入社申请','张三 申请加入您的社团，请前往成员管理审批。','join_request',2,0,'2026-07-18 22:18:38');
/*!40000 ALTER TABLE `notifications` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `registrations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `registrations` (
  `registration_id` int NOT NULL AUTO_INCREMENT,
  `activity_id` int NOT NULL,
  `user_id` int NOT NULL,
  `status` enum('registered','cancelled','attended','enrolled','checked_in') NOT NULL DEFAULT 'registered',
  `enrolled_at` datetime DEFAULT NULL,
  `checkin_time` datetime DEFAULT NULL,
  `registered_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `cancelled_at` datetime DEFAULT NULL,
  PRIMARY KEY (`registration_id`),
  UNIQUE KEY `uk_act_user` (`activity_id`,`user_id`),
  KEY `idx_user` (`user_id`),
  CONSTRAINT `registrations_ibfk_1` FOREIGN KEY (`activity_id`) REFERENCES `activities` (`activity_id`) ON DELETE CASCADE,
  CONSTRAINT `registrations_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=33 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `registrations` WRITE;
/*!40000 ALTER TABLE `registrations` DISABLE KEYS */;
INSERT INTO `registrations` VALUES (1,6,203,'enrolled',NULL,NULL,'2026-07-18 17:25:12',NULL),(2,8,203,'attended',NULL,'2026-07-18 17:27:31','2026-07-18 17:27:31',NULL),(3,8,287,'cancelled',NULL,NULL,'2026-07-18 20:34:52',NULL),(4,8,201,'registered',NULL,NULL,'2026-07-18 20:55:49',NULL),(5,10,101,'cancelled',NULL,NULL,'2026-07-18 22:20:58',NULL),(6,4,101,'cancelled',NULL,NULL,'2026-07-18 22:26:04',NULL),(32,13,101,'registered',NULL,NULL,'2026-07-19 10:54:45',NULL);
/*!40000 ALTER TABLE `registrations` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_registration_after_insert` AFTER INSERT ON `registrations` FOR EACH ROW BEGIN
    IF NEW.status = 'registered' THEN
        UPDATE activities SET current_count = current_count + 1 WHERE activity_id = NEW.activity_id;
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_registration_after_update` AFTER UPDATE ON `registrations` FOR EACH ROW BEGIN
    IF NEW.status = 'cancelled' AND OLD.status = 'registered' THEN
        UPDATE activities SET current_count = current_count - 1 WHERE activity_id = NEW.activity_id;
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
DROP TABLE IF EXISTS `reimbursement`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `reimbursement` (
  `reimbursement_id` int NOT NULL AUTO_INCREMENT,
  `club_id` int NOT NULL,
  `applicant_id` int NOT NULL,
  `amount` decimal(12,2) NOT NULL,
  `description` varchar(500) NOT NULL,
  `receipt_path` varchar(255) DEFAULT NULL,
  `status` enum('pending','approved','rejected') NOT NULL DEFAULT 'pending',
  `reviewer_id` int DEFAULT NULL,
  `review_comment` varchar(500) DEFAULT NULL,
  `submitted_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `reviewed_at` datetime DEFAULT NULL,
  PRIMARY KEY (`reimbursement_id`),
  KEY `applicant_id` (`applicant_id`),
  KEY `reviewer_id` (`reviewer_id`),
  KEY `idx_club` (`club_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `reimbursement_ibfk_1` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE,
  CONSTRAINT `reimbursement_ibfk_2` FOREIGN KEY (`applicant_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE,
  CONSTRAINT `reimbursement_ibfk_3` FOREIGN KEY (`reviewer_id`) REFERENCES `users` (`user_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `reimbursement` WRITE;
/*!40000 ALTER TABLE `reimbursement` DISABLE KEYS */;
INSERT INTO `reimbursement` VALUES (1,5,107,280.00,'算法书籍采购报销','/receipts/demo_receipt_01.jpg','approved',2,'审核通过，金额合理','2026-04-05 10:00:00','2026-04-06 14:00:00'),(2,6,103,150.00,'读书会茶歇购买报销','/receipts/demo_receipt_02.jpg','approved',3,'同意','2026-06-06 09:00:00','2026-06-07 11:00:00'),(3,1,101,450.00,'ACM新生赛奖品采购','/receipts/demo_receipt_03.jpg','pending',NULL,NULL,'2026-07-15 10:00:00',NULL),(4,4,108,600.00,'训练用足球采购','/receipts/demo_receipt_04.jpg','pending',NULL,NULL,'2026-07-16 14:00:00',NULL),(5,7,104,380.00,'趣味化学实验耗材采购','/receipts/demo_receipt_05.jpg','pending',NULL,NULL,'2026-07-17 09:00:00',NULL),(6,8,105,220.00,'创业大赛宣传物料','/receipts/demo_receipt_06.jpg','pending',NULL,NULL,'2026-07-17 10:00:00',NULL),(7,3,106,200.00,'望远镜配件采购','/receipts/demo_receipt_07.jpg','pending',NULL,NULL,'2026-07-17 11:00:00',NULL),(8,2,206,500.00,'志愿者马甲定制','/receipts/demo_receipt_08.jpg','rejected',3,'超出额度，请分批申请或缩减数量','2026-05-10 09:30:00','2026-05-10 09:30:00'),(9,1,101,100.00,'Test expense','receipts/1784366468_248195.txt','pending',NULL,NULL,'2026-07-18 17:21:08',NULL),(10,1,101,200.50,'Activity supplies purchase','receipts/1784366589_984492.txt','pending',NULL,NULL,'2026-07-18 17:23:09',NULL);
/*!40000 ALTER TABLE `reimbursement` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_reimbursement_after_update` AFTER UPDATE ON `reimbursement` FOR EACH ROW BEGIN
    IF NEW.status = 'approved' AND OLD.status = 'pending' THEN
        UPDATE reimbursement_limits
        SET current_period_used = current_period_used + NEW.amount
        WHERE club_id = NEW.club_id;
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
DROP TABLE IF EXISTS `reimbursement_limits`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `reimbursement_limits` (
  `limit_id` int NOT NULL AUTO_INCREMENT,
  `club_id` int NOT NULL,
  `single_limit` decimal(12,2) NOT NULL DEFAULT '0.00',
  `period_type` enum('monthly','semesterly') DEFAULT 'monthly',
  `period_limit` decimal(12,2) NOT NULL DEFAULT '0.00',
  `current_period_used` decimal(12,2) NOT NULL DEFAULT '0.00',
  `period_start` date DEFAULT NULL,
  `set_by` int DEFAULT NULL,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`limit_id`),
  UNIQUE KEY `uk_club` (`club_id`),
  KEY `set_by` (`set_by`),
  CONSTRAINT `reimbursement_limits_ibfk_1` FOREIGN KEY (`club_id`) REFERENCES `clubs` (`club_id`) ON DELETE CASCADE,
  CONSTRAINT `reimbursement_limits_ibfk_2` FOREIGN KEY (`set_by`) REFERENCES `users` (`user_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `reimbursement_limits` WRITE;
/*!40000 ALTER TABLE `reimbursement_limits` DISABLE KEYS */;
INSERT INTO `reimbursement_limits` VALUES (1,5,500.00,'monthly',2000.00,0.00,'2026-07-17',2,'2026-07-17 20:52:14'),(2,6,500.00,'monthly',2000.00,0.00,'2026-07-17',3,'2026-07-17 20:52:14'),(3,7,500.00,'monthly',2000.00,0.00,'2026-07-17',4,'2026-07-17 20:52:14'),(4,8,500.00,'monthly',2000.00,0.00,'2026-07-17',6,'2026-07-17 20:52:14'),(5,1,1000.00,'monthly',5000.00,0.00,'2026-07-17',1,'2026-07-17 20:52:14'),(6,2,1000.00,'monthly',5000.00,0.00,'2026-07-17',1,'2026-07-17 20:52:14'),(7,3,1000.00,'monthly',5000.00,0.00,'2026-07-17',1,'2026-07-17 20:52:14'),(8,4,1000.00,'monthly',5000.00,0.00,'2026-07-17',1,'2026-07-17 20:52:14');
/*!40000 ALTER TABLE `reimbursement_limits` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `system_config`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `system_config` (
  `config_id` int NOT NULL AUTO_INCREMENT,
  `config_key` varchar(100) NOT NULL,
  `config_value` text NOT NULL,
  `description` varchar(255) DEFAULT NULL,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`config_id`),
  UNIQUE KEY `config_key` (`config_key`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `system_config` WRITE;
/*!40000 ALTER TABLE `system_config` DISABLE KEYS */;
INSERT INTO `system_config` VALUES (1,'current_semester','2026-2027-1','','2026-07-18 17:30:00'),(2,'term_duration_days','365','社团任期时长（天）','2026-07-17 20:52:14'),(3,'election_notice_days','30','任期到期前提醒天数','2026-07-17 20:52:14'),(4,'auto_backup_hour','3','每日自动备份时刻（小时）','2026-07-17 20:52:14'),(5,'signin_code_length','6','活动签到码位数','2026-07-17 20:52:14'),(6,'student_no_length','10','学号位数','2026-07-17 20:52:14'),(7,'id_card_length','18','身份证号位数','2026-07-17 20:52:14'),(8,'phone_length','11','手机号位数','2026-07-17 20:52:14');
/*!40000 ALTER TABLE `system_config` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `user_restrictions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user_restrictions` (
  `restriction_id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `restriction_type` enum('ban_join','ban_activity') NOT NULL,
  `restricted_by` int NOT NULL,
  `start_time` datetime NOT NULL,
  `end_time` datetime DEFAULT NULL,
  `reason` varchar(500) DEFAULT NULL,
  `is_active` tinyint NOT NULL DEFAULT '1',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`restriction_id`),
  KEY `restricted_by` (`restricted_by`),
  KEY `idx_user` (`user_id`),
  KEY `idx_active` (`is_active`),
  CONSTRAINT `user_restrictions_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE,
  CONSTRAINT `user_restrictions_ibfk_2` FOREIGN KEY (`restricted_by`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `user_restrictions` WRITE;
/*!40000 ALTER TABLE `user_restrictions` DISABLE KEYS */;
INSERT INTO `user_restrictions` VALUES (1,275,'ban_join',1,'2026-07-01 00:00:00','2026-12-31 23:59:59','因上学期违规使用社团经费，禁止加入新社团',1,'2026-07-18 16:28:22'),(2,275,'ban_activity',1,'2026-07-01 00:00:00','2026-12-31 23:59:59','处罚期间同时限制参加活动',1,'2026-07-18 16:28:22'),(3,276,'ban_join',2,'2026-07-10 00:00:00','2026-08-09 23:59:59','恶意重复提交创建社团申请，限制创建社团权限30天',1,'2026-07-18 16:28:22');
/*!40000 ALTER TABLE `user_restrictions` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `user_id` int NOT NULL AUTO_INCREMENT,
  `username` varchar(50) NOT NULL,
  `password` varchar(64) NOT NULL,
  `real_name` varchar(50) NOT NULL,
  `role` enum('school_admin','college_admin','club_admin','club_member','general_student') NOT NULL DEFAULT 'general_student',
  `college_id` int DEFAULT NULL,
  `major_id` int DEFAULT NULL,
  `class_id` int DEFAULT NULL,
  `student_no` varchar(10) DEFAULT NULL,
  `id_card` varchar(18) DEFAULT NULL,
  `phone` varchar(11) DEFAULT NULL,
  `email` varchar(100) DEFAULT NULL,
  `status` tinyint NOT NULL DEFAULT '1',
  `last_login` datetime DEFAULT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `username` (`username`),
  KEY `major_id` (`major_id`),
  KEY `class_id` (`class_id`),
  KEY `idx_role` (`role`),
  KEY `idx_college` (`college_id`),
  KEY `idx_student_no` (`student_no`),
  CONSTRAINT `users_ibfk_1` FOREIGN KEY (`college_id`) REFERENCES `colleges` (`college_id`) ON DELETE SET NULL,
  CONSTRAINT `users_ibfk_2` FOREIGN KEY (`major_id`) REFERENCES `majors` (`major_id`) ON DELETE SET NULL,
  CONSTRAINT `users_ibfk_3` FOREIGN KEY (`class_id`) REFERENCES `classes` (`class_id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=288 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,'admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','学校管理员','school_admin',NULL,NULL,NULL,NULL,NULL,NULL,NULL,1,'2026-07-19 11:30:12','2026-07-17 20:52:14','2026-07-19 11:30:12'),(2,'cs_admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','信科院管理员','college_admin',1,NULL,NULL,NULL,NULL,NULL,NULL,1,'2026-07-18 21:46:07','2026-07-17 20:52:14','2026-07-18 21:46:07'),(3,'lit_admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','文学院管理员','college_admin',2,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(4,'sci_admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','理学院管理员','college_admin',3,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(5,'eng_admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','工学院管理员','college_admin',4,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(6,'biz_admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','经管院管理员','college_admin',5,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(7,'med_admin','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','医学院管理员','college_admin',6,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(101,'stu_zhang','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','张三','club_admin',1,1,1,'2023010001',NULL,'13800000001',NULL,1,'2026-07-19 10:42:57','2026-07-17 20:52:14','2026-07-19 10:42:57'),(102,'stu_li','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','李四','club_admin',1,2,3,'2023020001',NULL,'13800000002',NULL,1,'2026-07-18 17:12:38','2026-07-17 20:52:14','2026-07-18 17:12:38'),(103,'stu_wang','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','王五','club_admin',2,5,9,'2023050001',NULL,'13800000003',NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(104,'stu_zhao','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','赵六','club_admin',3,10,15,'2023100001',NULL,'13800000004',NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(105,'stu_qian','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','钱七','club_admin',5,14,19,'2023140001',NULL,'13800000005',NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(106,'stu_sun','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','孙八','club_admin',1,3,5,'2023030001',NULL,'13800000006',NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(107,'stu_zhou','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','周九','club_admin',1,4,7,'2023040001',NULL,'13800000007',NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(108,'stu_wu','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','吴十','club_admin',4,11,16,'2023110001',NULL,'13800000008',NULL,1,NULL,'2026-07-17 20:52:14','2026-07-17 20:52:14'),(201,'stu_cs01','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','刘明辉','club_admin',1,1,1,'2023010101',NULL,'13810000201',NULL,1,'2026-07-18 20:56:03','2026-07-18 16:29:27','2026-07-18 20:56:03'),(202,'stu_cs02','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','陈晓琳','general_student',1,1,1,'2023010102',NULL,'13810000202',NULL,1,'2026-07-18 16:55:29','2026-07-18 16:29:27','2026-07-18 16:55:29'),(203,'stu_cs03','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','黄志远','general_student',1,1,2,'2024010103',NULL,'13912345678','test@example.com',1,'2026-07-18 17:27:18','2026-07-18 16:29:27','2026-07-18 17:27:18'),(204,'stu_cs04','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','林雨婷','general_student',1,1,2,'2024010104',NULL,'13810000204',NULL,1,NULL,'2026-07-18 16:29:27','2026-07-18 16:29:27'),(205,'stu_cs05','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','张伟杰','general_student',1,2,3,'2023020105',NULL,'13810000205',NULL,1,NULL,'2026-07-18 16:29:27','2026-07-18 16:29:27'),(206,'stu_cs06','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','吴思琪','general_student',1,2,3,'2023020106',NULL,'13810000206',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(207,'stu_cs07','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','郑浩然','general_student',1,2,4,'2024020107',NULL,'13810000207',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(208,'stu_cs08','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','何美玲','general_student',1,3,5,'2023030108',NULL,'13810000208',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(209,'stu_cs09','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','赵子轩','general_student',1,3,5,'2023030109',NULL,'13810000209',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(210,'stu_cs10','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','孙晓萌','general_student',1,3,6,'2024030110',NULL,'13810000210',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(211,'stu_cs11','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','周逸飞','general_student',1,4,7,'2023040111',NULL,'13810000211',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(212,'stu_cs12','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','马思远','general_student',1,4,7,'2023040112',NULL,'13810000212',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(213,'stu_cs13','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','朱文博','general_student',1,4,8,'2024040113',NULL,'13810000213',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(214,'stu_cs14','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','胡雪晴','general_student',1,1,1,'2023010114',NULL,'13810000214',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(215,'stu_cs15','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','沈嘉豪','general_student',1,2,4,'2024020115',NULL,'13810000215',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(216,'stu_cs16','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','曹雅婷','general_student',1,3,6,'2024030116',NULL,'13810000216',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(217,'stu_cs17','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','唐志鹏','general_student',1,4,8,'2024040117',NULL,'13810000217',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(218,'stu_cs18','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','梁晓燕','general_student',1,1,2,'2024010118',NULL,'13810000218',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(219,'stu_cs19','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','许俊杰','general_student',1,2,3,'2023020119',NULL,'13810000219',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(220,'stu_cs20','240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9','高思源','general_student',1,3,5,'2023030120',NULL,'13810000220',NULL,1,NULL,'2026-07-18 16:30:19','2026-07-18 16:30:19'),(278,'new_student_01','8bf729f5f3e2ba07cb421f6046e008ef4958665133b14fded2c7271c4664525f','New Student','club_member',1,1,1,'2025010101','110101200001011234','13800000099',NULL,1,'2026-07-18 17:29:21','2026-07-18 17:29:14','2026-07-18 17:30:00'),(279,'test_cs','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试-信科院管理员','college_admin',1,NULL,NULL,NULL,NULL,NULL,NULL,1,'2026-07-18 19:18:34','2026-07-18 18:06:55','2026-07-18 19:18:34'),(280,'test_lit','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试-文学院管理员','college_admin',2,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-18 18:06:55','2026-07-18 18:06:55'),(281,'test_sci','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试-理学院管理员','college_admin',3,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-18 18:06:55','2026-07-18 18:06:55'),(282,'test_eng','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试-工学院管理员','college_admin',4,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-18 18:06:55','2026-07-18 18:06:55'),(283,'test_biz','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试-经管院管理员','college_admin',5,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-18 18:06:55','2026-07-18 18:06:55'),(284,'test_med','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试-医学院管理员','college_admin',6,NULL,NULL,NULL,NULL,NULL,NULL,1,NULL,'2026-07-18 18:06:55','2026-07-18 18:06:55'),(285,'test_student','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','测试学生','general_student',1,1,1,'2025888888',NULL,'13800008888',NULL,1,'2026-07-18 19:39:38','2026-07-18 18:06:55','2026-07-18 19:39:38'),(287,'12345','8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92','12345','club_member',1,1,1,'2541104213','','12345678969',NULL,1,'2026-07-18 21:06:59','2026-07-18 19:56:32','2026-07-18 21:34:20');
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `vote_logs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `vote_logs` (
  `id` int NOT NULL AUTO_INCREMENT,
  `election_id` int NOT NULL,
  `user_id` int NOT NULL,
  `voted_at` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_election_user` (`election_id`,`user_id`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `vote_logs_ibfk_1` FOREIGN KEY (`election_id`) REFERENCES `elections` (`election_id`) ON DELETE CASCADE,
  CONSTRAINT `vote_logs_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `vote_logs` WRITE;
/*!40000 ALTER TABLE `vote_logs` DISABLE KEYS */;
INSERT INTO `vote_logs` VALUES (1,1,201,'2026-03-03 10:00:00'),(2,1,203,'2026-03-03 10:05:00'),(3,1,208,'2026-03-03 10:10:00'),(4,1,211,'2026-03-03 10:15:00'),(5,1,214,'2026-03-03 10:20:00'),(6,1,216,'2026-03-03 10:25:00'),(7,1,219,'2026-03-03 10:30:00'),(8,1,233,'2026-03-03 10:35:00'),(9,1,235,'2026-03-03 10:40:00'),(10,1,239,'2026-03-03 10:45:00'),(11,1,243,'2026-03-03 10:50:00'),(12,1,255,'2026-03-03 10:55:00'),(13,1,202,'2026-03-03 11:00:00'),(14,1,205,'2026-03-03 11:05:00'),(15,3,201,'2026-07-12 10:00:00'),(16,3,204,'2026-07-12 10:05:00'),(17,3,213,'2026-07-12 10:10:00'),(18,3,218,'2026-07-12 10:15:00'),(19,3,229,'2026-07-12 10:20:00'),(20,3,246,'2026-07-12 10:25:00'),(21,3,252,'2026-07-12 10:30:00'),(22,3,269,'2026-07-12 10:35:00'),(23,2,202,'2026-07-18 16:52:46'),(24,2,203,'2026-07-18 16:53:00');
/*!40000 ALTER TABLE `vote_logs` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

