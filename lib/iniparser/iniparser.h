#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

typedef struct {
    struct list_head section_node;
    char *config_file;
} INI_CONFIG;

/**
 * @brief ini_config_create 读取ini配置保存到一个数据结构中
 * @param file      ini配置文件
 * @return  获取ini配置失败返回NULL, 否则返回一个保存ini配置的数据结构
 * @note    需要使用函数ini_config_release进行释放数据结构的内存
 */
extern INI_CONFIG *ini_config_create(const char *const file);

/**
 * @brief ini_config_get 获取指定字段的值
 * @param config    ini配置
 * @param section   字段所在的节, 可以指定NULL, 表示默认的节(没有节名)
 * @param key       字段对应的关键字
 * @param default_value 如果字段没有被设置, 则使用该值作为默认值
 * @return  返回字段的值
 */
extern const char *ini_config_get(const INI_CONFIG *config, const char *section,
    const char *key, const char *default_value);

/**
 * @brief ini_config_set 设置指定字段的值
 * @param config    ini配置
 * @param section   字段所在的节, 可以指定NULL, 表示默认的节(没有节名),
 *                  没有节名的节会被设置成配置中的第一节
 * @param key       字段的关键字
 * @param value     字段的设定值
 * @return  设置成功返回0, 设置失败返回-1
 */
extern int ini_config_set(INI_CONFIG *config, const char *section,
    const char *key, const char *value);

extern void *ini_config_get_section(const INI_CONFIG *config, const char *section);

extern const char *ini_config_get_key(const void *section, const  char *key,
        const char *default_value);

extern int ini_config_set_key(void *section, const char *key,
        const char *value);

/**
 * @brief ini_config_clear_section 清空配置中指定的节
 * @param config    ini配置
 * @param section   节, NULL表示无名节
 * @return  清空成功返回0, 失败返回-1
 */
extern int ini_config_clear_section(INI_CONFIG *config, const char *section);

/**
 * @brief ini_config_clear 清空配置
 * @param config    ini配置
 * @return  清空成功返回0, 失败返回-1
 */
extern int ini_config_clear(INI_CONFIG *config);

/**
 * @brief ini_config_erase_section 删除配置中指定的节
 * @param config    ini配置
 * @param section   节, NULL表示无名节
 * @return  删除成功返回0, 失败返回-1
 */
extern int ini_config_erase_section(INI_CONFIG *config, const char *section);

/**
 * @brief ini_config_erase_key 删除配置中指定的节中的关键字
 * @param config    ini配置
 * @param section   节, NULL表示无名节
 * @param key       关键字
 * @return  删除成功返回0, 失败返回-1
 */
extern int ini_config_erase_key(INI_CONFIG *config, const char *section,
    const char *key);

/**
 * @brief ini_copnfig_save2filestream 将ini配置保存到文件流中
 * @param config    ini配置
 * @param fp        文件流
 * @return  保存成功返回0, 失败返回-1
 */
extern int ini_config_save2filestream(INI_CONFIG *config, FILE *fp);

/**
 * @brief ini_config_saveas 将ini配置另存到另一个文件中
 * @param config    ini配置
 * @param file      保存ini配置的文件名
 * @return  保存成功返回0, 失败返回-1
 */
extern int ini_config_saveas(INI_CONFIG *config, const char *file);

/**
 * @brief ini_config_save 将ini配置保存到函数ini_config_create打开的文件中
 * @param config    ini配置
 * @return  保存成功返回0, 失败返回-1
 */
static inline int ini_config_save(INI_CONFIG *config) {
    return ini_config_saveas(config, config->config_file);
}

/**
 * @brief ini_config_release 释放ini数据结构的内存
 * @param config    ini配置
 */
extern void ini_config_release(INI_CONFIG *config);

#ifdef __cplusplus
}
#endif
