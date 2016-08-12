/**
 * @file c_ip_restrictor.h
 * @brief ip过滤类
 * @author baron baron@taomee.com
 * @version 1.0
 * @date 2011-02-23
 */
#ifndef __C_IP_RESTRICTOR_H__
#define __C_IP_RESTRICTOR_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * @brief 过滤规则结构体
 */
typedef struct {
    in_addr_t mask;
    in_addr_t ip;
} ip_rule_item_t;

typedef struct {
    int count;
    ip_rule_item_t item[0];
} ip_rule_t;

class c_ip_restrictor
{
    public:
        /**
         * @brief 构造函数
         */
        c_ip_restrictor();

        /**
         * @brief 拷贝构造函数
         *
         * @param copy 拷贝对象
         */
        c_ip_restrictor(const c_ip_restrictor &copy);

        /**
         * @brief 析构函数
         */
        ~c_ip_restrictor();

        /**
         * @brief 拷贝函数
         *
         * @param copy 拷贝对象
         *
         * @return 自身引用
         */
        c_ip_restrictor &operator=(const c_ip_restrictor &copy);

        /**
         * @brief 类对象初始化
         *
         * @param ip过滤规则字符串
         *
         * @return 0-有效 -1-无效
         */
        int init(const char *rule);

        /**
         * @brief 设置ip过滤规则
         *
         * @param ip过滤规则字符串
         *
         * @return 0-成功 -1-失败
         */
        int set_rule(const char *rule);

        /**
         * @brief 判断ip是否有效
         *
         * @param str_ip 待判断ip,字符串类型
         *
         * @return 0-有效 -1-无效
         */
        int is_valid(const char *str_ip);

        /**
         * @brief 判断ip是否有效
         *
         * @param str_ip 待判断ip,网络地址类型
         *
         * @return 0-有效 -1-无效
         */
        int is_valid(const in_addr_t *ip);

        /**
         * @brief 反初始化
         *
         * @return 0-成功 -1-失败
         */
        int uninit();

        ip_rule_t *get_rule() const;

    private:
        /**
         * @brief 是否初始化字段
         */
        int m_init_;

        ip_rule_t *m_p_rule_;

        /**
         * @brief ip地址是否有效(私有)
         *
         * @param ip 待判断ip,网络地址类型
         *
         * @return 0-有效 -1-无效
         */
        int is_valid_(const in_addr_t *ip);

        /**
         * @brief 设置ip过滤规则(私有)
         *
         * @param rule ip过滤规则字符串
         *
         * @return 0-成功 -1-失败
         */
        int set_rule_(const char *rule);
};

inline ip_rule_t *c_ip_restrictor::get_rule() const
{
    if (!m_init_)
    {
        return NULL;
    }

    return m_p_rule_;
}

#endif //!__C_IP_RESTRICTOR_H__
