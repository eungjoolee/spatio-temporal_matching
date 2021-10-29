//
// Created by 谢景 on 1/14/21.
//

#ifndef BOUNDING_BOX_MATCHING_YOLO_H
#define BOUNDING_BOX_MATCHING_YOLO_H
extern "C" {
#include "lide_c_util.h"
#include "lide_c_actor.h"
#include "lide_c_fifo.h"
#include "lide_c_fifo_basic.h"
}

#include <fstream>
#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"

#define YOLO_PROCESS 0


class yolo : public welt_cpp_actor{
public:
    yolo(lide_c_fifo_pointer out, int index);

    virtual ~yolo();

    int enable() override;

    void invoke() override;

    void reset() override;

    void connect(welt_cpp_graph *graph) override;
};


#endif //BOUNDING_BOX_MATCHING_YOLO_H
