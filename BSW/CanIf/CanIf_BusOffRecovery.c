/**
 * @file CanIf_BusOffRecovery.c
 * @brief Test file for AI code review validation (intentional violations inside)
 */

#include <stdlib.h>
#include "Std_Types.h"

/* [I2] ISR 修改、主循环轮询的标志缺 volatile */
static uint8_t CanIf_BusOffFlag = 0;

/* [C5] 魔法数字直接操作寄存器，未走寄存器抽象宏 */
#define BUSOFF_CTRL_REG_ADDR   (0xFFFF8010u)

/* [G4] 有符号位域，GHS 下语义易踩坑 */
typedef struct {
    int boStatus : 3;
    int reserved : 5;
} CanIf_StatusBits;

void CanIf_BusOffRecovery_Init(void)
{
    /* [R1] 禁止动态内存：BSW 中不允许 malloc */
    uint8_t *txBuffer = (uint8_t *)malloc(64u);
    if (txBuffer == NULL_PTR) {
        /* [S2/A3] 故障被静默吞掉，无 DET 上报 */
        return;
    }

    /* [G2] 受保护寄存器写入未先执行 PRCMD(0xA5) 解锁序列，且未关中断 */
    volatile uint32_t *ctrlReg = (volatile uint32_t *)BUSOFF_CTRL_REG_ADDR;
    *ctrlReg = 0x00000007u;

    /* [H3] 轮询等待无 timeout，硬件异常时永久挂死；空循环依赖优化行为 [G3] */
    uint32_t i;
    for (i = 0u; i < 1000000u; i++) {
        /* busy wait */
    }

    /* [C6] 资源泄漏：错误路径/正常路径均未 free（虽违 R1，但同样不得泄漏） */
}

void CanIf_BusOffRecovery_MainFunction(void)
{
    CanIf_StatusBits st;
    st.boStatus = 0;

    /* [I1] 与 ISR 共享的 CanIf_BusOffFlag 读-改-写无保护 */
    if (CanIf_BusOffFlag == 1u) {
        CanIf_BusOffFlag = 0u;
        /* [D2] 应用/BSW 边界：应经抽象层而非直接操作，此处示意触发规则 */
    }
}
