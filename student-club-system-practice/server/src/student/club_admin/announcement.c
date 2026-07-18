/**
 * announcement.c — 社团管理端 社团公告发布
 * 学生社团管理系统 SCMS（练习文件 — 代码空白，由你自行实现）
 *
 * 需要实现的处理函数：
 *
 *   void club_announce_list(ApiContext *ctx)
 *     路由：GET /api/club-admin/announcements
 *     参数：page, page_size
 *     功能：返回本社团公告列表
 *
 *   void club_announce_post(ApiContext *ctx)
 *     路由：POST /api/club-admin/announcements
 *     JSON body：title, content
 *     功能：发布公告，并自动推送通知给本社团所有成员
 *     操作步骤：
 *       1. INSERT INTO announcements(club_id, title, content, creator_id)
 *       2. 查询本社团所有在籍成员 user_id
 *       3. 批量 INSERT INTO notifications(user_id, type, title, content)
 *          type='club', title='公告：'+标题
 *
 *   void club_announce_delete(ApiContext *ctx)
 *     路由：DELETE /api/club-admin/announcements/{id}
 *     功能：删除公告
 *
 * 权限：需验证当前用户是该社团 president 或 vice_president
 *
 * 参考完整实现：student-club-system/src/student/club_admin/announcement.c
 */

#include "../../../include/api.h"
#include "../../../include/db.h"
#include "../../../include/common.h"
#include "../../../include/routes.h"
#include "../../../include/utils.h"

#include <stdio.h>
#include <string.h>

void club_announce_list(ApiContext *ctx) {
    /* TODO: 公告列表 */
}

void club_announce_post(ApiContext *ctx) {
    /* TODO: 发布公告并推送通知 */
}

void club_announce_delete(ApiContext *ctx) {
    /* TODO: 删除公告 */
}
