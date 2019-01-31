struct SolModule {

};

struct SolFunction {
    const char *name;
    struct SolType *parameterTypes;
    struct SolType *returnType;
    int paramCount;
};

struct SolType {

};

struct SolType SolI8Type;
struct SolType SolI16Type;
struct SolType SolI32Type;
struct SolType SolI64Type;

struct SolFunction *SolAddFunction(
    struct SolModule *m,
    const char *name,
    struct SolType *params,
    struct SolType *returnType
) {


    return NULL;
}
