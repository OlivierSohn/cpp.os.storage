
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
        
        enum class NChannels {
            ONE = 1,
            TWO = 2,
            MONO = 1,
            STEREO = 2
        };
        
        static WAVPCMHeader pcm_(int const num_frames, int const sample_rate,
                                 NChannels n_channels, int const bytes_per_sample) {
            auto const num_channels = to_underlying(n_channels);
            auto const size_data = num_frames * num_channels * bytes_per_sample;
            
            return {
                {'R','I','F','F'},
                static_cast<int32_t>(sizeof(WAVPCMHeader) + size_data),
                {'W','A','V','E'},
                {'f','m','t',' '},
                16,
                1,
                static_cast<int16_t>(num_channels),
                sample_rate,
                sample_rate * bytes_per_sample * num_channels,
                static_cast<int16_t>(bytes_per_sample * num_channels),
                static_cast<int16_t>(bytes_per_sample * 8),
                {'d','a','t','a'},
                size_data
            };
        }
        
        enum class SampleFormat {
            signed_16,
            signed_32
        };
        
        template<typename T>
        struct SignedSample;

        template<>
        struct SignedSample<int16_t> {
            static constexpr auto format = SampleFormat::signed_16;
        };
        
        template<>
        struct SignedSample<int32_t> {
            static constexpr auto format = SampleFormat::signed_32;
        };
        
        constexpr auto bytes_per_sample(SampleFormat f) {
            switch(f) {
                case SampleFormat::signed_16:
                    return 2;
                case SampleFormat::signed_32:
                    return 4;
            }
            return 0;
        }
        
        static WAVPCMHeader pcm(int num_frames, int sample_rate, NChannels n_channels, SampleFormat f) {
            return pcm_(num_frames, sample_rate, n_channels, bytes_per_sample(f));
        }
        
        struct WAVReader : public ReadableStorage {
            WAVReader(DirectoryPath const & d, FileName const & f) : ReadableStorage(d, f), header{} {}
            
            eResult Initialize();
            
            int countChannels() const { return header.num_channels; }
            int countSamples() const { return header.countSamples(); }
            int countFrames() const { return header.countFrames(); }
            int getSampleRate() const { return header.sample_rate; }
            float getDuration() const { return countFrames() / (float)getSampleRate(); }
            
            // 'it' is the begin of the buffer to write to, 'end' is the end.
            // returns the end element
            
            template<typename ITER>
            ITER Read(ITER it, ITER end) {
                while(it < end) {
                    A(audio_bytes_read < header.subchunk2_size);
                    constexpr auto n_reads = sizeof(decltype(*it));
                    
                    ReadData(&*it, n_reads, 1);
                    ++it;
                    audio_bytes_read += n_reads;
                    A(audio_bytes_read <= header.subchunk2_size);
                }
                return it;
            }

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

            template<typename T>
            void writeSample(T s) {
                WriteData(&s, sizeof(T), 1);
            }
            
        protected:
            
            void DoUpdateFileHeader() override;
            
        private:
            WAVPCMHeader header;
        };
    }
}
