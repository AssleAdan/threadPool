#include "Game.hpp"
#include "utils.hpp"


static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
Game::Game(game_params g) {
    this->m_gen_num=g.n_gen;
    this->interactive_on=g.interactive_on;
    this->print_on=g.print_on;
    this->m_filename=g.filename;
    //update effective threads num temporally
    this->m_thread_num=g.n_thread;
    pthread_mutex_init(&this->counter_mutex, nullptr);
    pthread_mutex_init(&this->hist_mutex, nullptr);
    pthread_cond_init(&counter_cond, nullptr);



    ///TODO: what about the hists? how should we save & update them?!
}

Game::~Game() = default;

void Game::run() {

    _init_game(); // Starts the threads and all other variables you need
    print_board("Initial Board");
    for (uint i = 0; i < m_gen_num; ++i) {
        auto gen_start = std::chrono::system_clock::now();
        _step(i,1); // Iterates a single generation

        _step(i,2); // Iterates a single generation

        auto gen_end = std::chrono::system_clock::now();
        m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
        print_board(nullptr);
    } // generation loop
    print_board("Final Board");
    _destroy_game();
}

void Game::_init_game() {

//////////
//build the board:

    // Create game fields - Consider using utils:read_file, utils::split
    vector<string> board = utils::read_lines(this->m_filename);
    this->m_field_height = board.size();
    this->m_board = new int_mat(this->m_field_height);
    this->m_next_board = new int_mat(this->m_field_height);

    for(uint i =0 ; i<board.size();++i){
        vector<string>  line = utils::split(board[i],' ');
        if(i == 0){
            this->m_field_width = line.size();
        }
        (*this->m_board)[i] =  vector<uint>(this->m_field_width);
        (*this->m_next_board)[i] =  vector<uint>(this->m_field_width);

        for(uint j = 0; j<this->m_field_width ; ++j){
            (*this->m_board)[i][j] = atoi(line[j].c_str());
            (*this->m_next_board)[i][j]=(*this->m_board)[i][j];
        }
    }
    /////////////////



    //update effective threads num
    if (this->m_field_height < this->m_thread_num) {
        this->m_thread_num = this->m_field_height;
    }


    //Create first m_thread_num threads with num_lines and start them
    for (uint k = 0; k < this->m_thread_num; k++) {

        Worker *thread = new Worker(k, this);
        this->m_threadpool.push_back(thread);
    }
    for (uint k = 0; k < this->m_thread_num; k++) {

        if (!this->m_threadpool[k]->start()){
            return;
        }
    }
}

void Game::_step(uint curr_gen, int phase) {

    // Push jobs to queue
    //let workers start the jobs
    this->counter=0;

    int thread_num_line = this->m_field_height/this->m_thread_num;
    int last_thread_lines = thread_num_line + this->m_field_height%this->m_thread_num;

    for (uint k = 0; k < this->m_thread_num - 1; k++) {
        job* new_job =new job(k * thread_num_line, thread_num_line, phase, curr_gen, this->m_gen_num);
        this->jobsQueue.push(new_job);
    }
    job* new_job = new job((this->m_thread_num - 1) * thread_num_line, last_thread_lines, phase, curr_gen, this->m_gen_num);
    this->jobsQueue.push(new_job);

    // Wait for the workers to finish calculating

    pthread_mutex_lock(&this->counter_mutex);
    while(this->counter != this->m_thread_num){
        pthread_cond_wait(&this->counter_cond,&this->counter_mutex);
    }
    pthread_mutex_unlock(&this->counter_mutex);

}

void Game::_destroy_game(){

    // Destroys board and frees all threads and resources
    // Not implemented in the Game's destructor for testing purposes.
    // All threads must be joined here
    for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[i]->join();
    }

    pthread_mutex_destroy(&this->counter_mutex);
    pthread_mutex_destroy(&this->hist_mutex);
    pthread_cond_destroy(&counter_cond);
    for (uint k = 0; k < this->m_thread_num; k++) {
        delete this->m_threadpool[k];
    }

   delete this->m_board;
   delete this->m_next_board;


}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

    if(print_on){

        // Clear the screen, to create a running animation
        if(interactive_on)
            system("clear");

        // Print small header if needed
        if (header != nullptr)
            cout << "<------------" << header << "------------>" << endl;

        cout << u8"╔" << string(u8"═") * this->m_field_width << u8"╗" << endl;
        for (uint i = 0; i < m_field_height; ++i) {
            cout << u8"║";
            for (uint j = 0; j < m_field_width; ++j) {
                if ((*this->m_board)[i][j] > 0)
                    cout << colors[(*this->m_board)[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
            }
            cout << u8"║" << endl;
        }
        cout << u8"╚" << string(u8"═") * this->m_field_width << u8"╝" << endl;

        // Display for GEN_SLEEP_USEC micro-seconds on screen
        if(interactive_on)
            usleep(GEN_SLEEP_USEC);
    }

}

const vector<double> Game::gen_hist() const {
    return this->m_gen_hist;
}

const vector<double> Game::tile_hist() const {
    return this->m_tile_hist;
}

uint Game::thread_num() const {
    return this->m_thread_num;
}


/*Function sketch to use for printing the board. You will need to decide its placement and how exactly
	to bring in the field's parameters. 

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/


job::job(uint start_index, uint lines_num, uint phase, uint curr_generation,uint generations_num) {
    this->lines_num = lines_num;
    this->start_index = start_index;
    this->phase = phase;
    this->curr_generation=curr_generation;
    this->generations_num=generations_num;
}

void job::updatePhase(uint phase)  {
    this->phase = phase;
}
uint job::getPhase() {
    return this->phase;
}

uint job::getStartIndex() {
    return this->start_index;
}

uint job::getWidth() {
    return this->lines_num;
}

job::~job() =default;


uint job::getCurrGen() {
    return this->curr_generation;
}

uint job::getGenNum() {
    return this->generations_num;
}

