#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    uint32_t lo  = (uint32_t)challenge;
    uint32_t hi  = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t a =(uint32_t)(3u * hi) ^ (uint32_t)(mid + 0xb7e3u);
    uint32_t b = (uint32_t)(3u * (uint32_t)(lo - hi) + 0x4a3bu);
    uint32_t ans = (uint32_t)(b + 2u * a) ^ 0xc9d2u;
    values[0]=ans;
    count=1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;

    /* v0 = 0x16 + 2 * ((challenge >> 31) & 0x1f) */
    uint32_t v = UINT32_C(0x16)
               + UINT32_C(2) * (uint32_t)((challenge >> 31) & UINT64_C(0x1f));
    values[0] = v;

    /* v_i = (((challenge >> (15 + 4*(i-1))) & 0x1f) ^ i) + 2*v_{i-1} */
    for (uint32_t i = 1; i <= 5; ++i) {
        unsigned shift = 15u + 4u * (i - 1u);
        uint32_t term = (uint32_t)((challenge >> shift) & UINT64_C(0x1f)) ^ i;
        v = term + UINT32_C(2) * v;
        values[i] = v;
    }

    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;

    uint32_t first = (uint32_t)challenge & UINT32_C(7);
    uint32_t arg   = (uint32_t)(challenge >> 32)
                   ^ (uint32_t)(challenge >> 3);

    static const int map[8] = {5, 6, 7, 2, 0, 4, 1, 3};

    values[0] = first;

    switch (map[first]) {
        case 0:
            values[1] = (arg ^ UINT32_C(0xc6ef)) + (arg >> 1);
            break;
        case 1:
            values[1] = UINT32_C(5) * arg + UINT32_C(0x1ff2c);
            break;
        case 2:
            values[1] = UINT32_C(3) * arg + UINT32_C(0x17c2);
            break;
        case 3:
            values[1] = (UINT32_C(2) * arg) ^ UINT32_C(0xd63a);
            break;
        case 4:
            values[1] = (arg ^ UINT32_C(0x73ef)) + (arg >> 2);
            break;
        case 5:
            values[1] = (UINT32_C(4) * arg) ^ UINT32_C(0x92dd);
            break;
        case 6:
            values[1] = UINT32_C(3) * arg + UINT32_C(0xb1c3);
            break;
        case 7:
            values[1] = UINT32_C(5) * arg + UINT32_C(0x353b3);
            break;
        default:
            values[1] = 0;
            break;
    }

    count = 2;
    return write_answer(out, cap, values, count);
}


static uint32_t p4_walk(uint32_t x, uint32_t low, uint32_t high, uint32_t c) {
    uint32_t mid = low + (high - low) / 2u;
    uint32_t m_plus_c = mid + c;

    if (x == mid) {
        return (mid + UINT32_C(232)) ^ c;
    } else if (x > mid) {
        uint32_t res = p4_walk(x, mid + 1u, high, c);
        return (UINT32_C(3) * res ^ m_plus_c) ^ UINT32_C(17);
    } else {
        uint32_t res = p4_walk(x, low, mid - 1u, c);
        return (UINT32_C(3) * res ^ m_plus_c) ^ UINT32_C(20);
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t x = (uint32_t)challenge & UINT32_C(0x7F);
    uint32_t c = (uint32_t)(challenge >> 7)
               ^ (uint32_t)(challenge >> 32);

    values[0] = p4_walk(x, 0u, 127u, c);
    count = 1;
    return write_answer(out, cap, values, count);
}


int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    // 节点数据表：{next索引, val, xor_val}
    static const struct {
        int next;
        uint32_t val;
        uint32_t xor_val;
    } nodes[] = {
        {-1,  819u,  23171u},   // 0: 0x405ba0
        { 8, 1330u,  62648u},   // 1: 0x405bb0
        { 5, 2064u,  34220u},   // 2: 0x405bd0
        { 7,  598u,  21608u},   // 3: 0x405be0
        { 9, 1806u,  19544u},   // 4: 0x405bf0
        { 6, 1597u,  29478u},   // 5: 0x405c00
        { 3, 2355u,  40681u},   // 6: 0x405c10
        { 1, 2896u,   1584u},   // 7: 0x405c20
        { 0, 2632u,  58828u},   // 8: 0x405c30
        { 2,  333u,   5945u},   // 9: 0x405c40
        { 4, 1139u,  44607u}    // 10: 0x405c50
    };

    // 8条链表的起始节点索引
    static const int head_idx[8] = {10, 4, 9, 2, 5, 6, 3, 7};

    // 1. 计算起始索引与遍历步数
    uint32_t idx   = (uint32_t)challenge & UINT32_C(7);
    uint32_t steps = ((uint32_t)(challenge >> 3)) & UINT32_C(3);

    // 2. 定位起始节点
    int cur = head_idx[idx];

    // 3. 初始累加值计算
    uint32_t shift6 = (uint32_t)(challenge >> 6);
    uint32_t high32 = (uint32_t)(challenge >> 32);
    uint32_t acc = (shift6 ^ high32) ^ nodes[cur].xor_val ^ UINT32_C(0x8016);
    acc += nodes[cur].val;
    uint32_t final_val = nodes[cur].val;

    // 4. 沿链表遍历 steps 步
    for (uint32_t i = 0; i < steps; i++) {
        cur = nodes[cur].next;
        acc ^= nodes[cur].xor_val;
        acc += nodes[cur].val;
        final_val = nodes[cur].val;
    }

    // 5. 输出口令：acc 在前，final_val 在后
    values[0] = acc;
    values[1] = final_val;
    count = 2;
    return write_answer(out, cap, values, count);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    // 1. 提取两级函数索引
    uint32_t idx1 = (uint32_t)challenge & UINT32_C(7);
    uint32_t idx2 = ((uint32_t)challenge >> 3) & UINT32_C(7);

    // 2. 计算第一级函数的入参
    uint32_t arg1 = (uint32_t)(challenge >> 6)
                  ^ (uint32_t)(challenge >> 32);

    // 3. 第一级间接调用：按 idx1 选函数运算
    uint32_t res1;
    switch (idx1) {
        case 0: case 2: case 4: // 三个索引复用 op0
            res1 = (arg1 + UINT32_C(0x4915)) ^ (arg1 >> 1);
            break;
        case 1: // op1：32位循环右移7位
            res1 = (arg1 >> 7) | (arg1 << 25);
            break;
        case 3: // op3：异或后叠加4倍原值
            res1 = (arg1 ^ UINT32_C(0xb5a9)) + 4u * arg1;
            break;
        case 5: // op4：循环左移3位后异或常量
            res1 = ((arg1 << 3) | (arg1 >> 29)) ^ UINT32_C(0x76cf);
            break;
        case 6: // op2：自身异或右移2位后加常量
            res1 = (arg1 ^ (arg1 >> 2)) + UINT32_C(0x7529);
            break;
        case 7: // op5：4倍加常量 异或 3倍值
            res1 = (4u * arg1 + UINT32_C(0xf807)) ^ (3u * arg1);
            break;
        default:
            res1 = 0;
            break;
    }

    // 4. 计算第二级函数的入参
    uint32_t arg2 = res1 ^ UINT32_C(0x3bd3);

    // 5. 第二级间接调用：按 idx2 选函数运算
    uint32_t res2;
    switch (idx2) {
        case 0: case 2: case 4:
            res2 = (arg2 + UINT32_C(0x4915)) ^ (arg2 >> 1);
            break;
        case 1:
            res2 = (arg2 >> 7) | (arg2 << 25);
            break;
        case 3:
            res2 = (arg2 ^ UINT32_C(0xb5a9)) + 4u * arg2;
            break;
        case 5:
            res2 = ((arg2 << 3) | (arg2 >> 29)) ^ UINT32_C(0x76cf);
            break;
        case 6:
            res2 = (arg2 ^ (arg2 >> 2)) + UINT32_C(0x7529);
            break;
        case 7:
            res2 = (4u * arg2 + UINT32_C(0xf807)) ^ (3u * arg2);
            break;
        default:
            res2 = 0;
            break;
    }

    // 6. 最终结果：第二级结果加常量
    uint32_t answer = res2 + UINT32_C(0xdb63);

    values[0] = answer;
    count = 1;
    return write_answer(out, cap, values, count);
}



static uint32_t secret_mix_solution(uint32_t x) {
    x = (x >> 1) | (x << 31);   /* ror $1 */
    return x ^ UINT32_C(0xb337);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t c   = (uint32_t)context & UINT32_C(3);

    uint32_t esi = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t r8d = (uint32_t)(context  >> 32) ^ (uint32_t)context;
    uint32_t edi = esi ^ r8d;

    uint32_t r9d = c * UINT32_C(0x1553);
    r9d ^= (uint32_t)(context >> 16);

    uint32_t eax = secret_mix_solution(edi) ^ r9d;

    uint32_t v0 = c;
    uint32_t v1 = eax;

    uint32_t edx = UINT32_C(0xa664) + r8d + c + (esi >> 1);
    uint32_t v2  = eax ^ edx;

    /* 32 字节填充 + "OPEN" + 三个整数，总长 >= 36 */
    int size = snprintf(out, cap,
                        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"  /* 32 字节 */
                        "OPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
                        v0, v1, v2);
    return size < 0 || (size_t)size >= cap;
}