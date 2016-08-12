#include <stdio.h>

#include "../../_ip_restrictor/c_ip_restrictor.h"

int main()
{
    c_ip_restrictor ip_restrictor;
    char rule[] = "192.168.1.1|192.168.3.4|192.168.1.159";

    int result = ip_restrictor.init(rule);

    if (result)
    {
        printf("\nerror\n");
    }

    printf("\n%d\n", ip_restrictor.is_valid("192.168.1.1"));
    printf("\n%d\n", ip_restrictor.is_valid("127.2.0.1"));

    in_addr_t addr = inet_network("192.168.1.158");
    printf("\n%d\n", ip_restrictor.is_valid(&addr));

    c_ip_restrictor dest = ip_restrictor;
    ip_rule_t *p_rule = dest.get_rule();

    for (int i = 0; i < p_rule->count; ++ i)
    {
        printf("\n%u %x\n", p_rule->item[i].ip, p_rule->item[i].mask);
    }

    return 0;
}
