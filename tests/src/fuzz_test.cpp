#include "ezp.h"
#include <queue>
#include <deque>
#include <random>
#include <functional>
#include <memory>
#include <bitset>
#include <locale.h>
#include <algorithm>



const int NUM_REPS = 10*1000*1000;
const int MAX_FLUSH_REPS = 1000*1000;

const int RETRY_INTVAL = 20;

const int MSG_SND_CHANCE = 100;
const int RECV_DROP_CHANCE = 50;

const int BYTE_RUDE_DROP_CHANCE = 10;
const int BYTE_NICE_DROP_CHANCE = 10;

const int BIT_ERROR_CHANCE = 150;
const int REPEAT_BIT_ERROR_CHANCE = 100;

const int MSG_SEND_SIDE_BIAS = 500;


typedef std::shared_ptr< std::array< ezp_master_t, 2 > > master_parr_t;

typedef std::function< int() > rand_prob_t;
typedef std::function< size_t() > rand_value_t;



enum lr_t {
    LEFT,
    RIGHT,
} ;

const char * lr_str(lr_t side){
    switch(side){
        case LEFT:
            return "LEFT";
        case RIGHT:
            return "RIGHT";
        default:
            return "<ERROR>";
    }
}

#define FOR_LR() for(lr_t side=LEFT; side < 2; side = static_cast<lr_t>(static_cast<int>(side)+1))

class Shitty_Connection {
public:


    Shitty_Connection(rand_prob_t r,  master_parr_t masters)
                : m_rand_prob(r)
                , m_masters(masters)
    {}



    uint8_t maybe_bit_error(uint8_t b){
        uint8_t old_b = b;
        if(m_rand_prob() < BIT_ERROR_CHANCE){
            b ^= (1 << (m_rand_prob()%8));


            while (m_rand_prob() < REPEAT_BIT_ERROR_CHANCE) {
                b ^= (1 << (m_rand_prob()%8));
            }
        }

        int count = std::bitset<8>(b ^ old_b).count();
        if(count == 1){
            b = old_b; // caught by parity check
        }
        if(b == old_b){
            m_bytes_sent_non_corrupted++;
        }else{
            EZP_LOG("----- c - %02x -> %02x\n", (int)old_b, (int)b);

            //int p = __builtin_popcount(b ^ old_b);
            int p = std::bitset<8>(b ^ old_b).count();
            m_bytes_sent_n_bits_corrupted[p]++;
        }

        return b;
    }


    EZP_RESULT write_byte(lr_t side, uint8_t byte){
        lr_t other = static_cast<lr_t>(!side);
        if(m_rand_prob() < BYTE_NICE_DROP_CHANCE){
            m_bytes_dropped_nice++;
            EZP_LOG("----- n -\n");
            return EZP_EAGAIN;
        }
        if(m_rand_prob() < BYTE_RUDE_DROP_CHANCE){
            m_bytes_dropped_rude++;
            EZP_LOG("----- r -\n");
            return EZP_OK;
        }
        byte = maybe_bit_error(byte);
        master_on_recv_byte(&(*m_masters)[other], byte);
        m_bytes_sent++;
        return EZP_OK;
    }


    EZP_RESULT flush(lr_t side){
        return EZP_OK;
    }

    void print_summary(){
        printf("bytes_dropped_nice: %'zu\n", m_bytes_dropped_nice);
        printf("bytes_dropped_rude: %'zu\n", m_bytes_dropped_rude);
        printf("bytes_sent: %'zu\n", m_bytes_sent);
        printf("bytes sent non-corrupted: %'zu\n", m_bytes_sent_non_corrupted);
        printf("bytes sent n bits corrupted: [");
        for(size_t b : m_bytes_sent_n_bits_corrupted){
            printf(" %zu",b);
        }
        printf("]\n");
    }

private:

    size_t m_bytes_sent_non_corrupted = 0;
    size_t m_bytes_sent_n_bits_corrupted[8] = {};

    size_t m_bytes_dropped_nice = 0;
    size_t m_bytes_dropped_rude = 0;
    size_t m_bytes_sent = 0;

    rand_prob_t m_rand_prob;
    master_parr_t m_masters;
};

class Tester;

struct PlatformVars {
    Shitty_Connection *con;
    lr_t side;
    Tester *tester;
};

EZP_RESULT on_recv_msg(void *usr_data, ezp_msg_t * msg) ;
EZP_RESULT write_byte(void *usr_data, uint8_t b) ;
EZP_RESULT flush(void *usr_data) ;



class Tester {
public:
    Tester(rand_prob_t rand_prob, rand_value_t rand_value)
                                : m_rand_prob(rand_prob)
                                , m_rand_value(rand_value)
                                , m_con(rand_prob, m_masters)
    {
        init();
    }


    void enqueue_rand_msg(){
        int r_msg_type = m_rand_prob();
        ezp_msg_t msg;
        if(r_msg_type < 300){
            msg.typeID = ezp_msgID_foo;
            msg.foo.a = m_rand_value() % 256;
            msg.foo.b = m_rand_value() % 256;

        }else if(r_msg_type < 400){
            msg.typeID = ezp_msgID_bar;
            msg.bar.c = m_rand_value() % 256;
            msg.bar.d = m_rand_value() % 256;
        }
        else if(r_msg_type < 500){
            msg.typeID = ezp_msgID_ping;
            msg.ping.val = m_rand_value() % 256;
        }
        else if(r_msg_type < 600){
            msg.typeID = ezp_msgID_pong;
            msg.ping.val = m_rand_value() % 256;
        }

        else if(r_msg_type < 700){
            msg.typeID = ezp_msgID_boop;
        }
        else if(r_msg_type < 800){
            msg.typeID = ezp_msgID_bop;
        }
        else {
            msg.typeID = ezp_msgID_big;
            msg.big.c = (char) m_rand_value();
            msg.big.d8 = (int8_t) m_rand_value();
            msg.big.d16 = (int16_t) m_rand_value();
            msg.big.d32 = (int32_t) m_rand_value();
            msg.big.d64 = (int64_t) m_rand_value();
            msg.big.u8 = (uint8_t) m_rand_value();
            msg.big.u16 = (uint16_t) m_rand_value();
            msg.big.u32 = (uint32_t) m_rand_value();
            msg.big.u64 = (uint64_t) m_rand_value();
        }

        lr_t side = (m_rand_prob() < MSG_SEND_SIDE_BIAS) ? LEFT : RIGHT;

        m_send_queue[side].push(msg);
    }


    void try_send_any_msgs(){
        FOR_LR(){
            lr_t other = static_cast<lr_t>(!side);
            if(m_send_queue[side].empty()){
                continue;
            }
            auto msg = m_send_queue[side].front();
            EZP_LOG("%s send: %d ...\n", lr_str(side), (int) msg.typeID);
            EZP_RESULT res = master_enqueue(&(*m_masters)[side], &msg);
            if(res == EZP_OK){
                EZP_LOG("                ... ok\n");
                m_sent[side].push_back(msg);
                m_send_queue[side].pop();
                m_msgs_sent++;
                m_total_sent_to[other]++;
            }else{
                EZP_LOG("                ... nope\n");
                m_msgs_not_sent++;
            }
        }
    }

    void run(){
        for(int i = 0; i < NUM_REPS; i++){
            if(i % (1000*1000) == 0){
                printf("rep: %'d\n", i);
            }
            FOR_LR(){
                EZP_LOG("%s PROCESS\n", lr_str(side));
                master_process(&(*m_masters)[side], 10);
                if(m_rand_prob() < MSG_SND_CHANCE){
                    enqueue_rand_msg();
                }
                try_send_any_msgs();
                check();
            }
        }

        m_flush_reps = 0;
        for( ; m_flush_reps < MAX_FLUSH_REPS && !all_sends_recvd(); ++m_flush_reps){
            FOR_LR(){
                EZP_LOG("%s EXTRA FLUSH PROCESS\n", lr_str(side));
                try_send_any_msgs();
                master_process(&(*m_masters)[side], 10);
            }
        }

        finish_check();
    }

    void print_results(){
        printf("\n------- R E S U L T S ----------- \n");
        printf("msgs sent: %'zu\n", m_msgs_sent);
        printf("MSGS NOT SENT (had to wait): %'zu\n", m_msgs_not_sent);
        printf("msgs dropped: %'zu\n", m_msgs_dropped);
        printf("msgs not dropped: %'zu\n", m_msgs_not_dropped);
        m_con.print_summary();
        printf("extra flush reps: %'zu\n", m_flush_reps);
        if(m_flush_reps == MAX_FLUSH_REPS){
            printf("FLUSHING DID NOT COMPLETE !!!\n");
        }
        printf("send queue remaining: L: %'zu, R: %'zu. Total, %'zu\n",
            m_send_queue[LEFT].size(), m_send_queue[RIGHT].size(),
            m_send_queue[LEFT].size() + m_send_queue[RIGHT].size());

        FOR_LR(){
            printf("%s\n", lr_str(side));
            printf("\tright: %'zu \n\twrong: %'zu \n\tnot recvd: %'zu \n\tsum: %'zu \n\ttotal sent to: %'zu\n",
                m_recvd_right[side], m_recvd_wrong[side], m_not_recvd[side],
                m_recvd_right[side] + m_recvd_wrong[side] + m_not_recvd[side],
                m_total_sent_to[side]);
        }

    }

    void check(){
        FOR_LR(){
            lr_t other = static_cast<lr_t>(!side);
            while(!m_recvd[side].empty()){
                bool found = false;
                for(int i=0; i< std::min<int>(5, m_sent[other].size()); i++){
                    if(ezp_msg_equal(&m_recvd[side].front(), & m_sent[other][i])){
                        // found
                        found = true;
                        // RECVD RIGHT
                        m_recvd_right[side]++;
                        m_recvd[side].pop_front();
                        // sent up to here not recvd
                        for(int j=0; j<i-1; j++){
                            m_sent[other].pop_front();
                            m_not_recvd[side]++;
                            // NOT RECVD
                        }
                        m_sent[other].pop_front();
                        break;
                    }
                }

                if(!found){
                    // RECVD WRONG
                    m_recvd_wrong[side]++;
                    m_recvd[side].pop_front();

                    if(m_sent[other].empty()){
                        m_not_recvd[side]--;
                    }else{
                        m_sent[other].pop_front();
                    }
                }
            }
        }
    }




    bool all_sends_recvd(){
        // check();
        FOR_LR(){
            if(!msgRingbuff_isEmpty(&(*m_masters)[side].m_send_queue)){
                return false;
            }
        }
        return true;
    }

    void finish_check(){
        check();

        FOR_LR(){
            lr_t other = static_cast<lr_t>(!side);
            while(!m_sent[side].empty()){
                m_sent[side].pop_front();
                // TEST(MSG_RECVD, false);
                m_not_recvd[other]++;
            }
        }
    }



    EZP_RESULT on_recv_msg(lr_t side, ezp_msg_t *msg){
        if(m_rand_prob() < RECV_DROP_CHANCE){
            m_msgs_dropped ++;
            return EZP_EAGAIN;
        }
        m_msgs_not_dropped++;
        m_recvd[side].push_back(*msg);
        return EZP_OK;
    }

    void init() {

        ezp_platform_t platforms[2];

        FOR_LR(){
            m_vars[side] = {
                .con = &m_con,
                .side = side,
                .tester = this
            };

            platforms[side] = {
                write_byte,
                flush,
                ::on_recv_msg,
                &m_vars[side]
            };

            master_init(  &(*m_masters)[side],
                        m_bytes[side], sizeof(m_bytes[0]),
                        m_msgs[side], countof(m_msgs[0]),
                        platforms[side], RETRY_INTVAL);
        }
    }


private:

    std::array< std::queue<ezp_msg_t>, 2 > m_send_queue;

    // int m_passed[TEST_TYPE_COUNT] = {0};
    // int m_failed[TEST_TYPE_COUNT] = {0};

    rand_prob_t m_rand_prob;
    rand_value_t m_rand_value;
    PlatformVars m_vars[2];

    // int m_tests_total = 0;
    // int m_tests_total_failed = 0;

    size_t m_msgs_sent = 0;
    size_t m_msgs_not_sent = 0;

    size_t m_msgs_dropped = 0;
    size_t m_msgs_not_dropped = 0;

    size_t m_flush_reps = 0;

    master_parr_t m_masters = std::make_shared < std::array<ezp_master_t, 2>>();

    uint8_t m_bytes[2][EZP_RECV_BUFFER_MIN_SIZE * 2];
    ezp_msg_t m_msgs[2][4];

    Shitty_Connection m_con;

    std::array<std::deque<ezp_msg_t>, 2> m_sent;
    std::array<std::deque<ezp_msg_t>, 2> m_recvd;

    std::array<size_t, 2> m_recvd_right = {0};
    std::array<size_t, 2> m_recvd_wrong = {0};
    std::array<size_t, 2> m_not_recvd = {0};
    std::array<size_t, 2> m_total_sent_to = {0};


};



EZP_RESULT flush(void *usr_data) {
    PlatformVars *vars = static_cast<PlatformVars *>(usr_data);

    return vars->con->flush(vars->side);
}

EZP_RESULT write_byte(void *usr_data, uint8_t b) {
    PlatformVars *vars = static_cast<PlatformVars *>(usr_data);
    return vars->con->write_byte(vars->side, b);
}



EZP_RESULT on_recv_msg(void *usr_data, ezp_msg_t * msg) {
    PlatformVars *vars = static_cast<PlatformVars *>(usr_data);
    return vars->tester->on_recv_msg(vars->side, msg);
}


int main(){
    setlocale(LC_NUMERIC, "");
    int seed = 110;
    printf("seed: %d\n", seed);
    std::default_random_engine rand_engine;
    rand_engine.seed(seed);
    std::uniform_int_distribution<int> prob_dist(0,1000);
    auto rand_prob = [&] {return prob_dist(rand_engine);};

    std::uniform_int_distribution<uint64_t> value_dist(0,(uint64_t)-1);
    auto value_prob = [&] {return value_dist(rand_engine);};

    Tester t(rand_prob, value_prob);
    t.run();
    t.print_results();
}