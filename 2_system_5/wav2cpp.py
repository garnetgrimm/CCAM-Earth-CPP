from scipy.io import wavfile
import os
from pathlib import Path
import re
import numpy as np

SAMPLES_DIR = Path('./samples/wav').resolve()
HEADERS_DIR = SAMPLES_DIR.parent / 'cpp'

HEADER_TEMPLATE = """
#include <array>

DSY_SDRAM_BSS std::array<uint16_t, {}> {} = {{
 {}
}};
"""

for input_fn in SAMPLES_DIR.iterdir():
    sample_name = f'{input_fn.stem}_sample'
    output_fn =  HEADERS_DIR / (input_fn.stem + ".h")
    samplerate, data = wavfile.read(input_fn)
    data = data.astype(np.uint16)

    data = ', '.join([f"{sample:-#0{6}x}" for sample in data])

    comma_number = 0
    def comma_newline(match):
        global comma_number
        comma_number += 1
        if (comma_number % 8) == 0:
            return ',\n'
        else:
            return ','

    data = re.sub(r',', comma_newline, data)
    
    result = HEADER_TEMPLATE.format(len(data), sample_name, data)
    with open(output_fn, 'w') as f:
        f.write(result)
    print(output_fn)