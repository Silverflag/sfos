/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2013 - 2018 Intel Corporation. */

/* Modeled on trace-events-sample.h */

/* The trace subsystem name for i40e will be "i40e".
 *
 * This file is named i40e_trace.h.
 *
 * Since this include file's name is different from the trace
 * subsystem name, we'll have to define TRACE_INCLUDE_FILE at the end
 * of this file.
 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM i40e

/* See trace-events-sample.h for a detailed description of why this
 * guard clause is different from most normal include files.
 */
#if !defined(_I40E_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _I40E_TRACE_H_

#include <linux/tracepoint.h>

/*
 * i40e_trace() macro enables shared code to refer to trace points
 * like:
 *
 * trace_i40e{,vf}_example(args...)
 *
 * ... as:
 *
 * i40e_trace(example, args...)
 *
 * ... to resolve to the PF or VF version of the tracepoint without
 * ifdefs, and to allow tracepoints to be disabled entirely at build
 * time.
 *
 * Trace point should always be referred to in the driver via this
 * macro.
 *
 * Similarly, i40e_trace_enabled(trace_name) wraps references to
 * trace_i40e{,vf}_<trace_name>_enabled() functions.
 */
#define _I40E_TRACE_NAME(trace_name) (trace_ ## i40e ## _ ## trace_name)
#define I40E_TRACE_NAME(trace_name) _I40E_TRACE_NAME(trace_name)

#define i40e_trace(trace_name, args...) I40E_TRACE_NAME(trace_name)(args)

#define i40e_trace_enabled(trace_name) I40E_TRACE_NAME(trace_name##_enabled)()

/* Events common to PF and VF. Corresponding versions will be defined
 * for both, named trace_i40e_* and trace_i40evf_*. The i40e_trace()
 * macro above will select the right trace point name for the driver
 * being built from shared code.
 */

#define NO_DEV "(i40e no_device)"

TRACE_EVENT(i40e_napi_poll,

	TP_PROTO(struct napi_struct *napi, struct i40e_q_vector *q, int budget,
		 int budget_per_ring, unsigned int rx_cleaned, unsigned int tx_cleaned,
		 bool rx_clean_complete, bool tx_clean_complete),

	TP_ARGS(napi, q, budget, budget_per_ring, rx_cleaned, tx_cleaned,
		rx_clean_complete, tx_clean_complete),

	TP_STRUCT__entry(
		__field(int, budget)
		__field(int, budget_per_ring)
		__field(unsigned int, rx_cleaned)
		__field(unsigned int, tx_cleaned)
		__field(int, rx_clean_complete)
		__field(int, tx_clean_complete)
		__field(int, irq_num)
		__field(int, curr_cpu)
		__string(qname, q->name)
		__string(dev_name, napi->dev ? napi->dev->name : NO_DEV)
		__bitmask(irq_affinity,	nr_cpumask_bits)
	),

	TP_fast_assign(
		__entry->budget = budget;
		__entry->budget_per_ring = budget_per_ring;
		__entry->rx_cleaned = rx_cleaned;
		__entry->tx_cleaned = tx_cleaned;
		__entry->rx_clean_complete = rx_clean_complete;
		__entry->tx_clean_complete = tx_clean_complete;
		__entry->irq_num = q->irq_num;
		__entry->curr_cpu = get_cpu();
		__assign_str(qname);
		__assign_str(dev_name);
		__assign_bitmask(irq_affinity, cpumask_bits(&q->affinity_mask),
				 nr_cpumask_bits);
	),

	TP_printk("i40e_napi_poll on dev %s q %s irq %d irq_mask %s curr_cpu %d "
		  "budget %d bpr %d rx_cleaned %u tx_cleaned %u "
		  "rx_clean_complete %d tx_clean_complete %d",
		__get_str(dev_name), __get_str(qname), __entry->irq_num,
		__get_bitmask(irq_affinity), __entry->curr_cpu, __entry->budget,
		__entry->budget_per_ring, __entry->rx_cleaned, __entry->tx_cleaned,
		__entry->rx_clean_complete, __entry->tx_clean_complete)
);

/* Events related to a vsi & ring */
DECLARE_EVENT_CLASS(
	i40e_tx_template,

	TP_PROTO(struct i40e_ring *ring,
		 struct i40e_tx_desc *desc,
		 struct i40e_tx_buffer *buf),

	TP_ARGS(ring, desc, buf),

	/* The convention here is to make the first fields in the
	 * TP_STRUCT match the TP_PROTO exactly. This enables the use
	 * of the args struct generated by the tplist tool (from the
	 * bcc-tools package) to be used for those fields. To access
	 * fields other than the tracepoint args will require the
	 * tplist output to be adjusted.
	 */
	TP_STRUCT__entry(
		__field(void*, ring)
		__field(void*, desc)
		__field(void*, buf)
		__string(devname, ring->netdev->name)
	),

	TP_fast_assign(
		__entry->ring = ring;
		__entry->desc = desc;
		__entry->buf = buf;
		__assign_str(devname);
	),

	TP_printk(
		"netdev: %s ring: %p desc: %p buf %p",
		__get_str(devname), __entry->ring,
		__entry->desc, __entry->buf)
);

DEFINE_EVENT(
	i40e_tx_template, i40e_clean_tx_irq,
	TP_PROTO(struct i40e_ring *ring,
		 struct i40e_tx_desc *desc,
		 struct i40e_tx_buffer *buf),

	TP_ARGS(ring, desc, buf));

DEFINE_EVENT(
	i40e_tx_template, i40e_clean_tx_irq_unmap,
	TP_PROTO(struct i40e_ring *ring,
		 struct i40e_tx_desc *desc,
		 struct i40e_tx_buffer *buf),

	TP_ARGS(ring, desc, buf));

DECLARE_EVENT_CLASS(
	i40e_rx_template,

	TP_PROTO(struct i40e_ring *ring,
		 union i40e_16byte_rx_desc *desc,
		 struct xdp_buff *xdp),

	TP_ARGS(ring, desc, xdp),

	TP_STRUCT__entry(
		__field(void*, ring)
		__field(void*, desc)
		__field(void*, xdp)
		__string(devname, ring->netdev->name)
	),

	TP_fast_assign(
		__entry->ring = ring;
		__entry->desc = desc;
		__entry->xdp = xdp;
		__assign_str(devname);
	),

	TP_printk(
		"netdev: %s ring: %p desc: %p xdp %p",
		__get_str(devname), __entry->ring,
		__entry->desc, __entry->xdp)
);

DEFINE_EVENT(
	i40e_rx_template, i40e_clean_rx_irq,
	TP_PROTO(struct i40e_ring *ring,
		 union i40e_16byte_rx_desc *desc,
		 struct xdp_buff *xdp),

	TP_ARGS(ring, desc, xdp));

DEFINE_EVENT(
	i40e_rx_template, i40e_clean_rx_irq_rx,
	TP_PROTO(struct i40e_ring *ring,
		 union i40e_16byte_rx_desc *desc,
		 struct xdp_buff *xdp),

	TP_ARGS(ring, desc, xdp));

DECLARE_EVENT_CLASS(
	i40e_xmit_template,

	TP_PROTO(struct sk_buff *skb,
		 struct i40e_ring *ring),

	TP_ARGS(skb, ring),

	TP_STRUCT__entry(
		__field(void*, skb)
		__field(void*, ring)
		__string(devname, ring->netdev->name)
	),

	TP_fast_assign(
		__entry->skb = skb;
		__entry->ring = ring;
		__assign_str(devname);
	),

	TP_printk(
		"netdev: %s skb: %p ring: %p",
		__get_str(devname), __entry->skb,
		__entry->ring)
);

DEFINE_EVENT(
	i40e_xmit_template, i40e_xmit_frame_ring,
	TP_PROTO(struct sk_buff *skb,
		 struct i40e_ring *ring),

	TP_ARGS(skb, ring));

DEFINE_EVENT(
	i40e_xmit_template, i40e_xmit_frame_ring_drop,
	TP_PROTO(struct sk_buff *skb,
		 struct i40e_ring *ring),

	TP_ARGS(skb, ring));

/* Events unique to the PF. */

#endif /* _I40E_TRACE_H_ */
/* This must be outside ifdef _I40E_TRACE_H */

/* This trace include file is not located in the .../include/trace
 * with the kernel tracepoint definitions, because we're a loadable
 * module.
 */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE i40e_trace
#include <trace/define_trace.h>