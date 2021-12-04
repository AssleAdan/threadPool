//
// Created by student on 1/3/21.
//

#include "Thread.hpp"
#include "Game.hpp"


Worker::Worker(int thread_id,Game* game):Thread(thread_id),game(game){
}

Worker::~Worker() = default;

int Worker::find_dominant(uint i, uint j) {
    int color[8] = {0};
    for(int l = -1 ; l < 2 ; l++) {
        for (int k = -1; k < 2; k++) {

            if (l == k && k == 0) continue;
            if (i + l < 0 || i + l >= this->game->m_field_height) continue;
            if (j + k < 0 || j + k >= this->game->m_field_width) continue;
            if ((*this->game->m_board)[i + l][j + k] == 0 ) continue;
            color[(*this->game->m_board)[i + l][j + k]]++;
        }
    }
    int result =  -1;
    int dominant= -1;
    for (int k = 1; k < 8; k++) {
        if (color[k]*k>dominant){
            dominant=color[k]*k;
            result = k;
        }
    }
    return result;
}

int  Worker::FindAverageAlive(uint i,uint j){
    double alive_neighbors = helper(i,j, this->game->m_next_board) + 1;
    if(alive_neighbors == 0) return 0;
    double sum = 0;
    for(int l = -1 ; l < 2 ;l++) {
        for (int k = -1; k < 2; k++) {
            
            if (i + l < 0 || i + l >= this->game->m_field_height) continue;
            if (j + k < 0 || j + k >= this->game->m_field_width) continue;
            sum += (*this->game->m_next_board)[i + l][j + k];
        }
    }
    int res = std::round((double)sum/alive_neighbors);
    return res;

}

int  Worker::helper(uint i,uint j,int_mat* mat) {
    int live_neighbors = 0;
    for(int l = -1 ; l < 2 ;l++) {
        for (int k = -1; k < 2; k++) {

            if (l == k && k == 0) continue;
            if (i + l < 0 || i + l >= this->game->m_field_height) continue;
            if (j + k < 0 || j + k >= this->game->m_field_width) continue;

            if ((*mat)[i + l][j + k] != 0)
                live_neighbors++;
        }
    }
    return live_neighbors;
}

void Worker::thread_workload() {

    while (1) {
        job* j = this->game->jobsQueue.pop();
        uint start_index = j->getStartIndex();
        uint lines_num = j->getWidth();
        int live_neighbors = 0;
		auto start = std::chrono::system_clock::now();

        //phase 1:
        if (j->getPhase() == 1) {
            
            for (uint i = start_index; i < start_index + lines_num; i++) {
                for (uint k = 0; k < this->game->m_field_width; k++) {
                    live_neighbors = helper(i, k,this->game->m_board);
                    if ((*this->game->m_board)[i][k] == 0 && live_neighbors == 3) {
                        (*this->game->m_next_board)[i][k] = this->find_dominant(i, k);
                    } else {
                        if ((*this->game->m_board)[i][k] > 0 && (live_neighbors == 2 || live_neighbors == 3)) {
                            (*this->game->m_next_board)[i][k] = (*this->game->m_board)[i][k];
                        } else {
                            (*this->game->m_next_board)[i][k] = 0;
                        }
                    }
                }
            }
      

        }
            //phase 2:
        else {
            for (uint i = start_index; i < start_index + lines_num; i++) {
                for (uint k = 0; k < this->game->m_field_width; k++) {
                    if ((*this->game->m_next_board)[i][k] > 0) {
                        (*this->game->m_board)[i][k] = this->FindAverageAlive(i, k);
                    } else {
                        (*this->game->m_board)[i][k] = 0;
                    }
                }
            }
            


        }




      auto end = std::chrono::system_clock::now();
      
        pthread_mutex_lock(&this->game->counter_mutex);
        this->game->m_tile_hist.push_back((double) std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        this->game->counter++;
        if (this->game->counter==this->game->thread_num()){
            pthread_cond_signal(&this->game->counter_cond);
        }
        pthread_mutex_unlock(&this->game->counter_mutex);


        if (j->getCurrGen() == j->getGenNum() - 1 && j->getPhase() == 2) {
            break;
        }
    }
}









