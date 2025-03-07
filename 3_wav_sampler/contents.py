HEADER_TEMPLATE = """

std::pair<uint16_t*, size_t> get_{sample_type_cpp}(uint8_t option) {{
    switch (option) {{
        case 0:
            return std::make_pair(
                TR606{sample_type_wav}01_sample.data(),
                TR606{sample_type_wav}01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606{sample_type_wav}02_sample.data(),
                TR606{sample_type_wav}02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606{sample_type_wav}03_sample.data(),
                TR606{sample_type_wav}03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606{sample_type_wav}04_sample.data(),
                TR606{sample_type_wav}04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606{sample_type_wav}05_sample.data(),
                TR606{sample_type_wav}05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606{sample_type_wav}_OD_sample.data(),
                TR606{sample_type_wav}_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }}
}}
"""

directory = {
    'Cymb': 'cymbol',
    'Hat_C': 'hat_closed',
    'Hat_O': 'hat_opened',
    'Hat_P': 'hat_pedal',
    'Kick': 'kick',
    'Snare': 'snare',
    'TomHi': 'tom_high',
    'TomLo': 'tom_low',
}

header = ""

for sample_type_cpp in directory.keys():
    for ext in ['01', '02', '03', '04', '05', '_OD']:
        header += f'#include "TR606{sample_type_cpp}{ext}.h"\n'

for sample_type_wav, sample_type_cpp in directory.items():
    header += HEADER_TEMPLATE.format(
        sample_type_cpp=sample_type_cpp,
        sample_type_wav=sample_type_wav
    )

with open('./samples/cpp/directory.h', 'w') as f:
    f.write(header)