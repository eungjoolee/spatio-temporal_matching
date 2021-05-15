//
// Created by 谢景 on 8/13/20.
//

#include <iostream>
#include "frame_simulator.h"
#define MAX_FIFO_COUNT 1

using namespace std;

frame_simulator::frame_simulator(char *file, int max_thread_num) {
    this->file = file;
    this->fp.open(this->file);
//    this->out = (welt_c_fifo_pointer) out;
    this->mode = FRAME_SIM_UPDATE;
    this->actor_set_max_port_count(MAX_FIFO_COUNT);
    frame_reader();
    this->frame_idx = 0;
    this->compute_idx = 0;
    this->current_obj_cnt = 0;
    this->max_thread_num = max_thread_num;
    this->thread_num = max_thread_num;
    this->data_out_size = 0;
}

bool frame_simulator::enable() {
    boolean result = FALSE;

    switch (mode) {
        case FRAME_SIM_UPDATE:
            result = true;
            break;
        case FRAME_SIM_PROCESS:
            result = true;
            break;
        case FRAME_SIM_OUTPUT:
            result = true;
            break;
        case FRAME_SIM_INACTIVE:
            result = FALSE;
            break;
        default:
            result = FALSE;
            break;
    }
    return result;
}

void frame_simulator::invoke() {
    switch (mode) {
        case FRAME_SIM_UPDATE: {
//            cout << "update" << endl;
//            if bounding box vector is not empty, get the max pair to be the
//            best matching
            if (!boundingBoxPairVec.empty()){
                int batch_size = frames[frame_idx-1].size();
                int batch_num = frames[frame_idx].size();
                auto max_pair = boundingBoxPairVec.begin();
                for (int j=0; j<batch_num; j++) {
                    double max_val = 0;
                    for (auto i = boundingBoxPairVec.begin() + j * batch_size;
                         i < boundingBoxPairVec.begin() + (j+1)*batch_size;
                         i++) {
                        if (max_val < i->result) {
                            max_val = i->result;
                            max_pair = i;
                        }
                    }
                    max_pair->dataVec[1]->setId(max_pair->dataVec[0]->getId());
                }

//                sort( boundingBoxPairVec.begin( ), boundingBoxPairVec.end( ),
//                        [ ](const Bounding_box_pair& lhs, const Bounding_box_pair& rhs )
//                {
//                    return lhs.result < rhs.result;
//                });
//                for (int i=0; i < frames[frame_idx-1].size(); ++i){
//                    auto it = boundingBoxPairVec.end()-(i+1);
//                    it->dataVec[1]->setId(it->dataVec[0]->getId());
//                }

//                cout << it->result << endl;
//                for (auto i : boundingBoxPairVec){
//                    i.output();
//                }
            }
//            if (frame_idx == 1){
//                for (auto i : frames[frame_idx])
//                    i.output();
//                exit(0);
//            }

//            if (frame_idx == frames.size()){
//                for (int k=0; k<frames.size(); k++) {
//                    for (auto i : frames[k])
//                        i.output();
//                }
//                exit(0);
//            }

//          reinitialize the bounding box vector
            boundingBoxPairVec.clear();
            for (int i=0; i < frames[frame_idx].size(); ++i) {
                for (int j = 0; j < frames[frame_idx + 1].size(); ++j) {
                    auto boundingBoxPair = Bounding_box_pair
                            (&frames[frame_idx][i],
                             &frames[frame_idx + 1][j]);
                    boundingBoxPairVec.push_back(boundingBoxPair);
                }
            }

//            if all the bounding box pairs are calculated, terminate
            frame_idx++;
            if (frame_idx == frames.size()-1) {
                mode = FRAME_SIM_OUTPUT;
                break;
            }

//            for (int i=0; i<frames[frame_idx].size(); ++i) {
//                for (int j = 0; j < frames[frame_idx+1].size(); ++j) {
//                    auto boundingBoxTriple = Bounding_box_triple
//                            (frames[frame_idx][i], frames[frame_idx+1][j],
//                             frames[frame_idx+2][k]);
//                    boundingBoxTripleVec.push_back(boundingBoxTriple);
//                }
//            }

//            cout << "update mode: " << endl;
//            for (auto i : boundingBoxTripleVec){
//                i.output();
//                cout << endl;
//            }
//            cout << endl;


//            if (frames[frame_idx].size()==frames[frame_idx+3].size()){
//                int group_size = boundingBoxTripleVec.size()/frames[frame_idx+3]
//                        .size();
//                for (int i=0; i<frames[frame_idx+3].size(); i++){
//                    for (int j=0; j<group_size; j++){
//                        boundingBoxTripleVec[i*group_size+j].update
//                        (frames[frame_idx+3][i]);
//                    }
//                }
//            } else if (frames[frame_idx].size() > frames[frame_idx+3].size()) {}

            mode = FRAME_SIM_PROCESS;
            break;
        }

//        distribute the task to fifos and let them go to the target actors. 
        case FRAME_SIM_PROCESS: {
//            cout << "process id: " << frame_idx << " curr: " <<
//            current_obj_cnt-1 << endl;
            data_out_size = boundingBoxPairVec.size();
            int data_out_cnt = 0;
            while (true) {
                for (int i = 0; i < thread_num; i++) {
                    if (data_out_cnt == data_out_size) {
                        goto MODE_SET;
                    }
                    auto data = &boundingBoxPairVec[data_out_cnt];
                    welt_c_fifo_write((welt_c_fifo_pointer) *
                            (this->portrefs[i]), &data);
                    data_out_cnt++;
                }
            }
            MODE_SET: mode = FRAME_SIM_UPDATE;
//            auto data = &boundingBoxPairVec[compute_idx];
//            welt_c_fifo_write(out, &data);
//            compute_idx++;
//            if (compute_idx == frames[frame_idx+1].size()) {
//                mode = FRAME_SIM_UPDATE;
//                compute_idx = 0;
//            } else {
//                mode = FRAME_SIM_PROCESS;
//            }
            break;
        }
        case FRAME_SIM_OUTPUT: {
            int frameid = 0;
            for (auto i : frames){
                cout << "frameid: " << ++frameid << endl;
                for (auto j : i){
                    j.output();
                }
                cout << endl;
            }
            mode = FRAME_SIM_INACTIVE;
        }
        case FRAME_SIM_INACTIVE:
            mode = FRAME_SIM_INACTIVE;
            break;
        default:
            mode = FRAME_SIM_INACTIVE;
            break;
    }
}

void frame_simulator::reset() {
    mode = FRAME_SIM_OUTPUT;
}

void frame_simulator::connect(welt_cpp_graph *graph) {

}

frame_simulator::~frame_simulator() {

}

void frame_simulator_terminator(frame_simulator *context) {
    delete context;
}

void frame_simulator::frame_reader() {
    int frame_idx_prev = 1;
    int bounding_box_id = 1;
    int frame_idx, left, top, width, height;
    vector<objData> frame;
    while (true) {
        if (!fp.is_open()) {
            cout << "cannot open file" << endl;
        }
        /* read line by line and convert to C string */
        string newline;
        getline(fp, newline);
        if (newline.empty()) {
            /* push the last frame */
            frames.push_back(frame);
            cout << "end of input" << endl;
            break;
        }
        const char *newCLine = newline.c_str();
        /* Parser */
        std::vector<char *> items;
        char *tmp_char_ptr = (char *) (newCLine);
        char *char_extracted = strtok(tmp_char_ptr, " ");
        while (char_extracted) {
//            std::cout << char_extracted << '\n';
            items.push_back(char_extracted);
            char_extracted = strtok(NULL, " ");
        }
        /* convert strings to numbers */

        sscanf(items[0], "%d", &frame_idx);
        sscanf(items[1], "%d", &left);
        sscanf(items[2], "%d", &top);
        sscanf(items[3], "%d", &width);
        sscanf(items[4], "%d", &height);

        if (frame_idx == frame_idx_prev){
            auto data = objData(bounding_box_id, left, top, width, height);
            bounding_box_id++;
            frame.push_back(data);
        } else if (frame_idx == frame_idx_prev+1){
            frame_idx_prev = frame_idx;
            frames.push_back(frame);
            frame.clear();
            bounding_box_id = 1;
            auto data = objData(bounding_box_id, left, top, width, height);
            bounding_box_id++;
            frame.push_back(data);
        } else {
            cout << "prev frame indicator error" << endl;
        }
    }
    int boxes = 0;
    for (auto i : frames){
        boxes+=i.size();
    }
    cout << "# frames: " << frames.size() << "; # boxes: " << boxes <<
         endl;
}
