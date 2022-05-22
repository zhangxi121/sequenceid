#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "sequenceid.h"

using namespace std;
using namespace nmsp;

/*
SequenceID的使用场景:
    保证消息的可靠性, 作为key来使用,
    (1).
    std::unordered_map<uint64_t, std::shared_ptr<yuanshuo::network::Package>> m_map_reqPkg;
    shared_ptr<yuanshuo::network::Package> pkgPtr = make_shared<Package>(len);
    pkgPtr->SetMessage(data, len);
    m_map_reqPkg[sequenceId] = pkgPtr;
    m_rwSocket->SendPackage(pkgPtr->GetBuffer(), pkgPtr->GetBufferLength());

    (2).
    Msg_Retry_SendMsg(uint64_t sequenceId)
    {
        uint8_t zero[6] = {0};
        if (memcmp(m_connid, zero, 6) == 0)
        {
            auto iter = m_map_reqPkg.find(sequenceId);
            if (iter != m_map_reqPkg.end())
            {
                std::shared_ptr<yuanshuo::network::Package> pkg = iter->second;
                m_rwSocket->SendPackage(pkg->GetBuffer(), pkg->GetBufferLength());
            }
        }
        return 0;
    }

    (3).
    AddTimeEvent(sequence_id, GetMillsecDur(EventmgrTimerType::CHECK_MSG_ACK),
                         std::bind(&EventMgrWorker::Msg_Retry_SendMsg, this, sequence_id), true);


    (4).
    void EventMgrWorker::EraseRequestPkg(const uint64_t sequenceId)
    {
        return m_map_reqPkg.erase(sequenceId);
    }

    bool EventMgrWorker::EraseTimeEvent(const uint32_t timerId)
    {
        return m_timerevent.cancelTimeEvent(timerId);
    }

    (5).
        EraseTimeEvent(message->GetAckId());              // 从定时器中删除,
        EraseRequestPkg(message->GetAckId());             // 从 m_map_reqPkg 中删除,
        m_pSeqId->ReleaseSequenceID(message->GetAckId()); // 从 sequence 中删除,



    为了保证消息的可靠性,  m_map_reqPkg[sequenceId] = pkgPtr, 使用 shared_ptr 共享对象,
    当收到 ackid 之后,  EraseTimeEvent(message->GetAckId());
                        EraseRequestPkg(message->GetAckId());
                        m_pSeqId->ReleaseSequenceID(message->GetAckId());
    ----------
*/

int main001(int argc, char const *argv[])
{
    // SequenceID::CreateInstance(2001, 3000); 桶比较多的时候,
    SequenceID::CreateInstance(2001, 3000);
    SequenceID *seqId = SequenceID::GetInstance();
    uint32_t sequenceId = 0;

    if (seqId == nullptr)
    {
        return 0;
    }

    for (int i = 0; i < 20; i++)
    {
        sequenceId = seqId->GenSequenceID();
        cout << sequenceId << endl;
    }
    if (!seqId->ReleaseSequenceID(2902))
    {
        cout << __FILE__ << " " << __LINE__ << endl;
    }
    if (!seqId->ReleaseSequenceID(3502))
    {
        cout << __FILE__ << " " << __LINE__ << endl;
    }
    cout << "------------------" << endl;
    for (int i = 0; i < 10; i++)
    {
        sequenceId = seqId->GenSequenceID();
        cout << sequenceId << endl;
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    SequenceID::CreateInstance(2001, 3000);
    uint32_t sequenceId = 0;

    auto thread_task = [&]()
    {
        SequenceID *seqId = SequenceID::GetInstance();
        for (int i = 0; i < 20; i++)
        {
            sequenceId = seqId->GenSequenceID();
            cout << sequenceId << endl;
        }
    };

    for (int i = 0; i < 10; i++)
    {
        thread(thread_task).join();
    }

    return 0;
}

/*
2001
2301
2601
2901
3201
3501
3801
4101
4401
4701
2002
2302
2602
2902
3202
3502
3802
4102
4402
4702
*/
