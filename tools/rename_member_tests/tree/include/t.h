/* Fixture types: two structs share the member name unk_04. */
typedef struct {
    /* 0x00 */ int unk_00;
    /* 0x04 */ int unk_04;              /* renamed; Foo_t.unk_04 in a comment too */
} Foo_t;

typedef struct {
    /* 0x00 */ Foo_t foo[2];
    /* 0x10 */ int unk_04;              /* keep: another type's unk_04 */
} Bar_t;

extern Bar_t bars[2];
