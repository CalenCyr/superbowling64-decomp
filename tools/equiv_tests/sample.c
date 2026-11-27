/* Self-test input for tools/equiv.py: one function in three spellings. VARIANT 0 is the reference, VARIANT 1
   an equivalent rewrite (other registers and loop shape), VARIANT 2 a planted bug (a wrong value only when a
   callee returns 3). */
typedef struct {
    int count;
    short values[8];
    float scale;
} sample_t;

extern int Sample_Get( int index );
extern void Sample_Put( int index, int value, float scaled );
extern int sample_total;

int Sample_Run( sample_t *s, int n ) {
#if VARIANT == 1
    int k = 0;
    int v;

    while ( k < n && k < 8 ) {
        v = Sample_Get( k );
        s->values[k] = v;
        sample_total = sample_total + v;
        Sample_Put( k, v, v * s->scale );
        k++;
    }
    s->count = k;
    return sample_total;
#else
    int i;
    int v;

    for ( i = 0; i < n && i < 8; i++ ) {
        v = Sample_Get( i );
#if VARIANT == 2
        if ( v == 3 ) {
            v = 4;
        }
#endif
        s->values[i] = v;
        sample_total += v;
        Sample_Put( i, v, s->scale * v );
    }
    s->count = i;
    return sample_total;
#endif
}
