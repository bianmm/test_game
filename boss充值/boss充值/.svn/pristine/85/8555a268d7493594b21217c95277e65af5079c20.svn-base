/**
 * @file c_ip_restrictor.cpp
 * @brief ip过滤类
 * @author baron baron@taomee.com
 * @version 1.0
 * @date 2011-02-22
 */

#include "../_ip_restrictor/c_ip_restrictor.h"
#include <stdlib.h>
#include <string.h>

c_ip_restrictor::c_ip_restrictor()
    :m_p_rule_(NULL),
     m_init_(0)
{
}

c_ip_restrictor::c_ip_restrictor(const c_ip_restrictor &copy)
    :m_p_rule_(NULL),
     m_init_(0)
{
    *this = copy;
}

c_ip_restrictor::~c_ip_restrictor()
{
    uninit();
}

c_ip_restrictor &c_ip_restrictor::operator=(const c_ip_restrictor &copy)
{
    ip_rule_t *p_rule = copy.get_rule();

    if (p_rule)
    {
        if (m_init_)
        {
            uninit();
        }

        int size = sizeof(ip_rule_t) + p_rule->count * sizeof(ip_rule_item_t);
        m_p_rule_ = reinterpret_cast<ip_rule_t *>(malloc(size));
        memcpy(m_p_rule_, p_rule, size);
        m_init_ = 1;
    }

    return *this;
}

int c_ip_restrictor::uninit()
{
    if (!m_init_)
    {
        return -1;
    }

    free(m_p_rule_);
    m_p_rule_ = NULL;
    m_init_ = 0;
    return 0;
}

int c_ip_restrictor::init(const char *rule)
{
    if (m_init_)
    {
        return -1;
    }

    if (!rule)
    {
        return -1;
    }

    if (set_rule_(rule))
    {
        return -1;
    }

    m_init_ = 1;
    return 0;
}

int c_ip_restrictor::set_rule(const char *ip_rule)
{
    if (!m_init_)
    {
        return -1;
    }

    if (!ip_rule)
    {
        return -1;
    }

    return set_rule_(ip_rule);
}

int c_ip_restrictor::set_rule_(const char *ip_rule)
{
    char *p_save;
    char *token;

    int count = 0;
    int ret = 0;

    ip_rule_t *rule2;
    char *tmp = strdup(ip_rule);

    while (1)
    {
        if (0 == count)
        {
            if (!strtok_r(tmp, "|", &p_save))
            {
                break;
            }
        }
        else
        {
            if (!strtok_r(NULL, "|", &p_save))
            {
                break;
            }
        }

        ++ count;
    }

    if (0 == count)
    {
        ret = -1;
        goto out;
    }

    rule2 = reinterpret_cast<ip_rule_t *>(
                malloc(sizeof(ip_rule_t) + count * sizeof(ip_rule_item_t)));
    rule2->count = count;
    token = tmp;

    for (int i = 0, next = 0; i < count; ++ i)
    {
        token += next;
        next = strlen(token) + 1;
        char *p_slash = index(token, '/');

        if (p_slash)
        {
            int shift = atoi(p_slash + 1);

            if (shift > 31 || shift < 1)
            {
                free(rule2);
                ret = -1;
                goto out;
            }

            rule2->item[i].mask = 0xFFFFFFFF << shift;
            *p_slash = '\0';
        }
        else
        {
            rule2->item[i].mask = 0xFFFFFFFF;
        }

        rule2->item[i].ip = inet_network(token);

        if (-1 == rule2->item[i].ip)
        {
            free(rule2);
            ret = -1;
            goto out;
        }
    }

    if (m_p_rule_)
    {
        free(m_p_rule_);
    }

    m_p_rule_ = rule2;

out:
    free(tmp);
    return ret;
}

int c_ip_restrictor::is_valid(const in_addr_t *ip)
{
    if (!m_init_)
    {
        return -1;
    }

    if (!ip)
    {
        return -1;
    }

    return is_valid_(ip);
}

int c_ip_restrictor::is_valid(const char *str_ip)
{
    if (!m_init_)
    {
        return -1;
    }

    in_addr_t ip = -1;

    if (!str_ip || (-1 == (ip = inet_network(str_ip))))
    {
        return -1;
    }

    return is_valid_(&ip);
}

int c_ip_restrictor::is_valid_(const in_addr_t *ip)
{
    for (int i = 0; i < m_p_rule_->count; ++ i)
    {
        if (((*ip) & m_p_rule_->item[i].mask)
                == (m_p_rule_->item[i].ip & m_p_rule_->item[i].mask))
        {
            return 0;
        }
    }

    return -1;
}
