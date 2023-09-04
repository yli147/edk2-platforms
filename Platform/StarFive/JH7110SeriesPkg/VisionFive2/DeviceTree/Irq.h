/** @file
  This header provide all the interrupt device tree bindings

  Copyright (c) 2023, StarFive Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __DT_BINDINGS_INTERRUPT_CONTROLLER_IRQ_H__
#define __DT_BINDINGS_INTERRUPT_CONTROLLER_IRQ_H__

#define IRQ_TYPE_NONE         0
#define IRQ_TYPE_EDGE_RISING  1
#define IRQ_TYPE_EDGE_FALLING 2
#define IRQ_TYPE_EDGE_BOTH    (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH   4
#define IRQ_TYPE_LEVEL_LOW    8

#endif /* __DT_BINDINGS_INTERRUPT_CONTROLLER_IRQ_H__ */
