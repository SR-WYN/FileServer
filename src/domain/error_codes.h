#pragma once

enum ErrorCodes
{
    SUCCESS = 0,
    ERROR_JSON = 1001,          // JSON 解析错误
    TOKEN_MISSING = 1002,       // 缺少 Token
    TOKEN_INVALID = 1003,       // Token 无效
    UID_MISMATCH = 1004,        // uid 与 Token 不匹配
    FILE_TOO_LARGE = 1005,      // 文件过大
    FILE_TYPE_INVALID = 1006,   // 文件类型不支持
    FILE_SAVE_FAILED = 1007,    // 文件保存失败
    FILE_NOT_FOUND = 1008,      // 文件不存在
    DB_UPDATE_FAILED = 1009,    // 数据库更新失败
};
