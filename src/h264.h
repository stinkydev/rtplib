#pragma once

#include <vector>
#include <stdint.h>
#include <iostream>

namespace h264 {

static size_t next_start_code (const uint8_t* data, size_t size)
{
  /* Boyer-Moore string matching algorithm, in a degenerative
   * sense because our search 'alphabet' is binary - 0 & 1 only.
   * This allow us to simplify the general BM algorithm to a very
   * simple form. */
  /* assume 1 is in the 3th byte */
  size_t offset = 2;

  while (offset < size) {
    if (1 == data[offset]) {
      auto shift = offset;

      if (0 == data[--shift]) {
        if (0 == data[--shift]) {
          return shift + 3;
        }
      }
      /* The jump is always 3 because of the 1 previously matched.
       * All the 0's must be after this '1' matched at offset */
      offset += 3;
    } else if (0 == data[offset]) {
      /* maybe next byte is 1? */
      offset++;
    } else {
      /* can jump 3 bytes forward */
      offset += 3;
    }
    /* at each iteration, we rescan in a backward manner until
     * we match 0.0.1 in reverse order. Since our search string
     * has only 2 'alpabets' (i.e. 0 & 1), we know that any
     * mismatch will force us to shift a fixed number of steps */
  }
  std::cout << "Cannot find next NAL start code. returning " <<  size << std::endl;

  return size;
}

static std::vector<size_t> get_nal_offsets(const uint8_t* data, size_t size) {
  std::vector<size_t> offsets;
  uint8_t next_offset = 0;

  while (next_offset < size) {
    const auto offset = next_start_code(data + next_offset, size);
    if (offset == size) {
      break;
    }
    offsets.push_back(next_offset + offset);
    next_offset += offset;
  } 
   

  return offsets;
}

} // namespace h264