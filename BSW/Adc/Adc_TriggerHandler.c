/**
 * @file Adc_TriggerHandler.c
 * @brief ADC 触发处理（第二个 AI 审查测试文件）
 * @attention 故意埋入违规点，用于验证 AI Code Review
 */

#include "Std_Types.h"

/* ADC 转换结果寄存器（故意：魔法数字 + 绕过 Mcal 抽象层） */
#define ADC_RESULT_REG (*(uint32 *)0xFFFF8120u)

static uint16 Adc_LastValue = 0u;
static uint8 Adc_GroupState = 0u;

/* 违规点1：中断函数未用 #pragma interrupt 声明（G1），
 * 也未声明 EI level / 寄存器 bank */
void Adc_Group0Notification(void)
{
    uint32 regVal;

    /* 违规点2：隐式类型截断 —— 32 位寄存器值直接塞进 16 位变量，
     * 高位数据丢失且编译器可能仅给 warning（C1） */
    Adc_LastValue = ADC_RESULT_REG;

    /* 违规点3：在 ISR（临界）上下文里调用非快返函数，
     * 拉长中断响应时间（I3） */
    regVal = Adc_DoHeavyConversion(Adc_LastValue);
    (void)regVal;

    Adc_GroupState = 1u;
}

/* 违规点4：BSW 上层模块直接读外设寄存器，绕过 Adc（Mcal）
 * 分层边界（A1/H1） */
uint16 Adc_GetRawDirect(void)
{
    /* 又一个魔法数字直访 */
    return (uint16)(*(volatile uint32 *)0xFFFF8124u);
}

/* 违规点5：安全相关报文（ASIL-B 采样值）转发前未做 E2E/CRC
 * 保护，且无超时监测（S1） */
void Adc_ForwardSafetyStream(uint8 *dest, uint32 len)
{
    uint32 i;
    for (i = 0u; i < len; i++)
    {
        /* 违规点6：无边界/超时保护的搬运循环，dest 越界即踩内存（H3/R2） */
        dest[i] = (uint8)(Adc_LastValue >> (i & 7u));
    }
    /* 无 CRC、无 sequence counter、无 alive counter —— 静默转发 */
}

/* 违规点7：周期函数硬编码假设 10ms 调用周期，未做漂移防护（A5） */
void Adc_TriggerMainFunction(void)
{
    if (Adc_GroupState == 1u)
    {
        /* 依赖上一次 ISR 到现在恰好 10ms 的隐含时序假设 */
        Adc_GroupState = 0u;
        Adc_LastValue = Adc_LastValue + 1u; /* 补偿假设的固定漂移 */
    }
}
