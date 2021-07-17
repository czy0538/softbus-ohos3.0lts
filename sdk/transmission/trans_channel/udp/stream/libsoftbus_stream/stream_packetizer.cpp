/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stream_packetizer.h"

#include "common_inner.h"
#include "securec.h"
#include "stream_common.h"
#include "stream_packet_header.h"


#define LOG_TAG "STREAM_PACKETIZER"

namespace Communication {
namespace SoftBus {
ssize_t StreamPacketizer::CalculateHeaderSize() const
{
    auto total = StreamPacketHeader::STREAM_HEADER_SIZE;

    return total;
}

ssize_t StreamPacketizer::CalculateExtSize(ssize_t size) const
{
    auto total = 0;

    // 如果携带额外信息，需要计算二级TLV大小, 此处暂时只需要传输extbuf
    if (size > 0) {
        total += TwoLevelsTlv::HEADER_LEN + TwoLevelsTlv::NUMS_LEN; // 1级头
        total += TwoLevelsTlv::HEADER_LEN + TwoLevelsTlv::NUMS_LEN; // 2级头
        total += Align(size, SHIFT);                                // 四字节对齐数据
        total += TwoLevelsTlv::CHECK_SUM_LEN;                       // 校验尾
    }

    return total;
}

std::unique_ptr<char[]> StreamPacketizer::PacketizeStream()
{
    dataSize_ = originData_->GetBufferLen();
    hdrSize_ = CalculateHeaderSize();
    extSize_ = CalculateExtSize(originData_->GetExtBufferLen());
    auto data = std::make_unique<char[]>(hdrSize_ + extSize_ + dataSize_);

    auto streamPktHeader = StreamPacketHeader(streamType_, extSize_ > 0, originData_->GetSeqNum(),
        originData_->GetStreamId(), extSize_ + dataSize_);
    streamPktHeader.Packetize(data.get(), hdrSize_, 0);

    TwoLevelsTlv tlv(originData_->GetExtBuffer(), originData_->GetExtBufferLen());
    if (tlv.Packetize(data.get(), extSize_, hdrSize_) != 0) {
        LOG_ERR("packetize tlv failed");
        return nullptr;
    }

    LOG_INFO("streamPktHeader version = %d, subVersion = %d, extFlag = %d, streamType = %d, marker = %d, flag = %d, "
        "streamId = %d (%x), timestamp = %u (%x), dataLen = %u (%x), seqNum = %d (%x), subSeqNum = %d (%x), "
        "dataSize_ = %zd, extSize_ = %zd",
        streamPktHeader.GetVersion(), streamPktHeader.GetSubVersion(), streamPktHeader.GetExtFlag(),
        streamPktHeader.GetStreamType(), streamPktHeader.GetMarker(), streamPktHeader.GetFlag(),
        streamPktHeader.GetStreamId(), streamPktHeader.GetStreamId(), streamPktHeader.GetTimestamp(),
        streamPktHeader.GetTimestamp(), streamPktHeader.GetDataLen(), streamPktHeader.GetDataLen(),
        streamPktHeader.GetSeqNum(), streamPktHeader.GetSeqNum(), streamPktHeader.GetSubSeqNum(),
        streamPktHeader.GetSubSeqNum(), dataSize_, extSize_);

    LOG_INFO("TLV version: %d, num = %d, extSize = %zd, extLen = %zd, checksum = %u",
        tlv.GetVersion(), tlv.GetTlvNums(), extSize_, tlv.GetExtLen(), tlv.GetCheckSum());

    auto ret = memcpy_s(data.get() + hdrSize_ + extSize_, dataSize_, originData_->GetBuffer().get(),
        originData_->GetBufferLen());
    if (ret != 0) {
        LOG_ERR("Failed to memcpy data!, ret:%d", ret);
    }

    return data;
}
} // namespace SoftBus
} // namespace Communication
