#include "../include/dma_type.h"
#include "../include/dma_mmu.h"
// #include <iostream>

#include "src_olddefine.h"
#include "../include/dma_type.h"

namespace dma_memsch
{

#define sd_c2v(v) ((v >= '0' && v <= '9') ? (v - '0') : (v - 'A' + 10))
    int sd_toval(const char *vs)
    {
        int hi = sd_c2v(vs[0]);
        int lo = sd_c2v(vs[1]);

        return hi * 0x10 + lo;
    }
    char sd_toupper(char v)
    {
        if (v >= 'a' && v <= 'z')
        {
            v -= 0x20;
        }
        return v;
    }
    struct sd_skip
    {
        int offset;
        unsigned char cmap[256];
    };
    // #define sd_setcmap(m,p)(m[p/8]|=(1<<((p%8))))
    // #define sd_iscmap(m,p)( (m[p/8]>>(p%8)) & 0b00000001)
#define sd_setcmap(m, p) (m[p] = 1)
#define sd_iscmap(m, p) (m[p] == 1)
    int sd_convert(const char *pattern, unsigned short *pat)
    {
        int len = strlen(pattern);

        char val[3];
        val[2] = 0;

        int val_idx = 0;

        bool flag_fuzzy = false;
        bool flag_char = false;
        int pat_idx = 0;

        bool flag_cvt = false;
        for (int i = 0; i < len + 1; i++)
        {
            if (i == len)
                flag_cvt = true;
            else
            {
                char v = sd_toupper(pattern[i]);
                if ((v >= '0' && v <= '9') || (v >= 'A' && v <= 'Z'))
                {
                    if (flag_fuzzy)
                        return 0;
                    if (val_idx >= 2)
                        return 0;
                    val[val_idx++] = v;
                    flag_char = true;
                }
                else if (v == '?')
                {
                    if (flag_char)
                        return 0;
                    flag_fuzzy = true;
                }
                else if (v == ' ')
                    flag_cvt = true;
                else
                    return 0;
            }

            if (flag_cvt)
            {
                if (flag_fuzzy && flag_char)
                    return 0;
                if (flag_fuzzy)
                {
                    pat[pat_idx++] = 0x100;
                    flag_fuzzy = false;
                }
                else if (flag_char)
                {
                    if (val_idx != 2)
                        return false;
                    pat[pat_idx++] = sd_toval(val);
                    flag_char = false;
                }

                val_idx = 0;

                flag_cvt = false;
            }
        }
        return pat_idx;
    }
    int sd_praseskip(const unsigned short *pat, int pat_len, sd_skip *skip)
    {
        int prev_v = 0;
        for (int i = 0; i < pat_len; i++)
        {
            if ((pat[i] == 0x100 && prev_v != 0x100))
            {
                skip[0].offset = i;
                return 1;
            }
            else
                sd_setcmap(skip[0].cmap, pat[i]);
            prev_v = pat[i];
        }
        return 0;
    }
    u64 sd_dosearch(
        mmu_fast *fast,
        uptr buf, u64 buf_len,
        const unsigned short *pat, int pat_len,
        sd_skip *skip, int skip_cnt)
    {
        u64 i = 0;
        u64 match_size = 0;

        int pat_8b_idx = 0;
        int pat_idx = 0;
        bool flag_notmatch = false;

        mmu_cache c1, c2;

        while (i < buf_len)
        {
            byte bufv = 0;
            ;
            if (
                fast->read_with_cache(&c1, buf + i, &bufv, 1) &&
                (pat[pat_idx] == 0x100 || bufv == pat[pat_idx]))
            {
                match_size++;
                i++;
                pat_idx++;
            }
            else
            {
                flag_notmatch = true;
            }

            if (flag_notmatch)
            {
                int skip_v = 1;

                u64 next_idx = i - match_size + skip[0].offset;
                unsigned char next_v = 0;
                if (fast->read_with_cache(&c2, buf + next_idx, &next_v, 1))
                {
                    if (!sd_iscmap(skip[0].cmap, next_v))
                        skip_v = skip[0].offset;
                }
                i = i - match_size + skip_v;

                match_size = 0;
                pat_8b_idx = 0;
                pat_idx = 0;
                flag_notmatch = false;
            }
            if (match_size == pat_len)
                return i - match_size;
        }

        return -1;
    }
    u64 sd_dosearch_noskip(
        mmu_fast *fast,
        uptr buf, u64 buf_len,
        const unsigned short *pat, int pat_len)
    {
        bool cmap[256];
        memset(cmap, 0, sizeof(cmap));
        for (int i = 0; i < pat_len; i++)
        {
            cmap[pat[i]] = 1;
        }
        int prev_sz = pat_len;

        u64 i = 0;
        u64 match_size = 0;

        int pat_8b_idx = 0;
        int pat_idx = 0;
        bool flag_notmatch = false;
        mmu_cache c1, c2;

        while (i < buf_len)
        {
            byte bufv = 0;
            ;
            // printf("i:%p\r\n", buf + i);
            if (fast->read_with_cache(&c1, buf + i, &bufv, 1) &&
                (pat[pat_idx] == 0x100 || bufv == pat[pat_idx]))
            {
                match_size++;
                i++;
                pat_idx++;
            }
            else
            {
                flag_notmatch = true;
            }
            if (flag_notmatch)
            {
                int skip_v = 1;
                u64 next_idx = i - match_size + prev_sz;

                byte next_v = 0;
                if (fast->read_with_cache(&c2, buf + next_idx, &next_v, 1))
                {
                    if (cmap[next_v] == 0)
                    {
                        skip_v = prev_sz + 1;
                    }
                }

                i = i - match_size + skip_v;

                match_size = 0;
                pat_8b_idx = 0;
                pat_idx = 0;
                flag_notmatch = false;
            }
            if (match_size == pat_len)
                return i - match_size;
        }

        return -1;
    }
    int sd_lstrip(const unsigned short *pat, int pat_len)
    {
        int start = 0;
        for (int i = 0; i < pat_len; i++)
        {
            if (pat[i] == 0x100)
                start++;
            else
                break;
        }
        return start;
    }
    int sd_rstrip(const unsigned short *pat, int pat_len)
    {

        int off = 0;
        for (int i = pat_len - 1; i >= 0; i--)
        {
            if (pat[i] == 0x100)
                off++;
            else
                break;
        }
        return off;
    }
    u64 sd_search(mmu_fast *fast, uptr buf, u64 buf_len, const char *pattern)
    {
        unsigned short cmp_pat[200];
        int cmp_pat_len = sd_convert(pattern, cmp_pat);

        int left = sd_lstrip(cmp_pat, cmp_pat_len);
        int right = sd_rstrip(cmp_pat, cmp_pat_len);

        sd_skip skip[1];
        memset(skip, 0, sizeof(skip));
        int skip_cnt = sd_praseskip(cmp_pat + left, cmp_pat_len - left - right, skip);

        u64 result = 0;
        if (skip_cnt == 0)
            result = sd_dosearch_noskip(fast, buf, buf_len, cmp_pat + left, cmp_pat_len - left - right);
        else
            result = sd_dosearch(fast, buf, buf_len, cmp_pat + left, cmp_pat_len - left - right, skip, skip_cnt);
        return result != -1 ? (result - left) : -1;
    }
    int sd_patlen(const char *pattern)
    {
        unsigned short cmp_pat[200];
        int cmp_pat_len = sd_convert(pattern, cmp_pat);
        return cmp_pat_len;
    }
}
