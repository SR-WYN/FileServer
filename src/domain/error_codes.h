#pragma once

enum ErrorCodes
{
    SUCCESS = 0,
    ERROR_JSON = 1001,          // JSON 解析错误
    RPCFAILED = 1002,           // RPC 请求错误
    TOKEN_MISSING = 1003,       // 缺少 Token
    TOKEN_INVALID = 1004,       // Token 无效
    UID_INVALID = 1005,         // uid 无效
    UID_MISMATCH = 1006,        // uid 与 Token 不匹配
    FILE_TOO_LARGE = 1007,      // 文件过大
    FILE_TYPE_INVALID = 1008,   // 文件类型不支持
    FILE_SAVE_FAILED = 1009,    // 文件保存失败
    FILE_NOT_FOUND = 1010,      // 文件不存在
    DB_UPDATE_FAILED = 1011,    // 数据库更新失败
};
