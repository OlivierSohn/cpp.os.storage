
namespace imajuscule {
    namespace audio {
        
        // http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
        
        struct WAVPCMHeader {
            char chunk_id[4];
            int chunk_size;
            char format[4];
            char subchunk1_id[4];
            int subchunk1_size;
            short int audio_format;
            short int num_channels;
            int sample_rate;            // sample_rate denotes the sampling rate.
            int byte_rate;
            short int block_align;
            short int bits_per_sample;
            char subchunk2_id[4];
            int subchunk2_size; // subchunk2_size denotes the number of samples.
            
            unsigned int countFrames() const {
                if(0 == block_align) {
                    // header has not been read yet
                    return 0;
                }
                A(0 == subchunk2_size % block_align);
                A(block_align);
                return subchunk2_size / block_align;
            }
            
            unsigned int countSamples() const {
                return countFrames() * num_channels;
            }
            
            unsigned int getSampleSize() const {
                A(bits_per_sample % 8 == 0);
                return bits_per_sample / 8;
            }
        };
        
        static WAVPCMHeader pcm_16b_stereo(int num_frames, int sample_rate) {
            constexpr auto bytes_per_sample = 2;
            constexpr auto num_channels = 2;
            auto size_data = num_frames * num_channels * bytes_per_sample;
            
            WAVPCMHeader res {
                {'R','I','F','F'},
                static_cast<int>(sizeof(WAVPCMHeader) + size_data),
                {'W','A','V','E'},
                {'f','m','t',' '},
                16,
                1,
                num_channels,
                sample_rate,
                sample_rate * bytes_per_sample * num_channels,
                4,
                bytes_per_sample * 8,
                {'d','a','t','a'},
                size_data
            };
            return res;
        }
        struct WAVReader : public ReadableStorage {
            WAVReader(DirectoryPath const & d, FileName const & f) : ReadableStorage(d, f), header{} {}
            
            eResult Initialize();
            
            int countChannels() const { return header.num_channels; }
            int countSamples() const { return header.countSamples(); }
            int countFrames() const { return header.countFrames(); }
            int getSampleRate() const { return header.sample_rate; }
            float getDuration() const { return countFrames() / (float)getSampleRate(); }
            
            using InterleavedAudio = std::vector<short int>;
            using ITER = InterleavedAudio::iterator;
            
            // 'it' is the begin of the buffer to write to, 'end' is the end.
            // returns the end element
            ITER Read(ITER it, ITER end);
            
            bool HasMore() const;
            
        private:
            WAVPCMHeader header;
            unsigned int audio_bytes_read = 0;
            
            bool readHeader();
        };
        
        struct WAVWriter : public WritableStorage {
            WAVWriter(DirectoryPath d, FileName f, WAVPCMHeader h) : WritableStorage(d,f), header(h) {}

            ~WAVWriter() { Finalize(); }
            
            eResult Initialize() {
                return doSaveBegin();
            }
            
            void writeSample(signed short);
        protected:
            
            void DoUpdateFileHeader() override;
            
        private:
            WAVPCMHeader header;
        };
    }
}
