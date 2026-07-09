#ifndef COM_TYPE_H
#define COM_TYPE_H

/*----------------------------------------------------------------------------
 * 全局通用基础类型定义
 * 说明：统一项目内基础数据类型，所有模块优先包含本文件
 *---------------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

/* 布尔类型 */
typedef uint8_t bool_t;

#ifndef TRUE
#define TRUE    (1U)
#endif

#ifndef FALSE
#define FALSE   (0U)
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

#endif /* COM_TYPE_H */
