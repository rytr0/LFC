#include <stdlib.h>
#include <stdio.h>
#include "interpreter.h"
#include "y.tab.h"

// Hold ex() return value.
// NULL if no return value (eg declaration, assignment).
typedef conNodeType ret;

nodeType * _ONE = NULL;
nodeType * ONE() {
    if (_ONE != NULL) return _ONE;
    _ONE = (nodeType*)xmalloc(sizeof(nodeType));

    _ONE->type = nodeCon;
    _ONE->con = *(conNodeType *)xmalloc(sizeof(conNodeType));
    _ONE->con.i = 1;
    _ONE->con.r = 1.0;
    _ONE->con.b = 1;

    return _ONE;
}


/* Execute a subtree.
 * Return node evaluation.
 */
ret * ex(nodeType *p) {
    ret * retv = NULL;

    // Early exit on EOTree.
    if (!p) return 0;

    switch(p->type) {
        case nodeCon:
            // assert conNodeType
            retv = xmalloc(sizeof(ret));
            retv->type = p->con.type;
            switch(p->con.type){
                case INTTYPE:
                    retv->i = p->con.i;
                    break;
                case REALTYPE:
                    retv->r = p->con.r;
                    break;
                case BOOLTYPE:
                    retv->b = p->con.b;
                    break;
                default:
                    yyerror("Unrecognized type.");
            }

            return retv;

        case nodeId:
            {
                symrec * s = getsym(p->id.name);
                if(s == NULL){
                    fprintf(stderr, "There is not such '%s' variable in the symtable\n", p->id.name);
                    exit(1);
                }

                retv = xmalloc(sizeof(ret));
                retv->type = s->type;
                switch(s->type){
                    case INTTYPE:
                        retv->i = s->i;
                        break;
                    case REALTYPE:
                        retv->r = s->r;
                        break;
                    case BOOLTYPE:
                        retv->b = s->b;
                        break;
                    default:
                        yyerror("Unrecognized type.");
                }

                return retv;
            }

        case nodeOpr:
            switch(p->opr.oper) {
                case WHILE:
                    while(ex(p->opr.op[0]))
                        ex(p->opr.op[1]);
                    return NULL;

                case FOR:
                    {
                        /* 0: var
                         * 1: initial value
                         * 2: upper boundary
                         * 3: body
                         */
                        // var = exp;
                        // TODO: test me
                        // symrec * s = getsym(p->opr.op[0]->id.name);
                        //ex(opr(EQ, 2, id(s), p->opr.op[1]));
                        ex(opr(EQ, 2, p->opr.op[0], p->opr.op[1]));

                        // iterator < boundary
                        while(ex(opr(LTE, 2, p->opr.op[0], p->opr.op[2]))){
                            // exec
                            ex(p->opr.op[3]);
                            // inc
                            ex(opr(PLUS, 2, p->opr.op[0], ONE()));
                        }
                        return 0;
                    }

                case IF:
                    if (ex(p->opr.op[0]))
                        ex(p->opr.op[1]); // IF
                    else if (p->opr.nops > 2)
                        ex(p->opr.op[2]); // ELSE (if any)
                    return 0;


                case PRINT:
                    {
                        ret * to_print = ex(p->opr.op[0]);
                        switch(to_print->type){
                            case INTTYPE:
                                printf("%d\n", to_print->i);
                                break;
                            case REALTYPE:
                                printf("%f\n", to_print->r);
                                break;
                            case BOOLTYPE:
                                if (to_print->b)
                                    printf("true\n");
                                else
                                    printf("false\n");
                                break;
                            default:
                                yyerror("Unrecognized type.");
                        }
                        return 0;
                    }

                case SEMICOLON:
                    ex(p->opr.op[0]);
                    return ex(p->opr.op[1]);

                case EQ:
                    {
                        symrec * s = getsym(p->opr.op[0]->id.name);
                        if(s == NULL){
                            fprintf(stderr, "There is not such '%s' varibale in the symtable\n", p->opr.op[0]->id.name);
                            exit(0);
                        }

                        ret * val = ex(p->opr.op[1]);

                        // TODO: INTRODUCE HERE coercion & type checking

                        s->type = val->type;
                        switch(val->type){
                            case INTTYPE:
                                s->i = val->i;
                                break;
                            case REALTYPE:
                                s->r = val->r;
                                break;
                            case BOOLTYPE:
                                s->b = val->b;
                                break;
                            default:
                                yyerror("Unrecognized type.");
                        }

                        return val;
                    }

                // TODO: introduce here coercion
                case UMINUS:
                    return -ex(p->opr.op[0]);// TODO: fixme

                case PLUS:
                    {
                        ret * a = ex(p->opr.op[0]);
                        ret * b = ex(p->opr.op[1]);

                        // TODO: add here type checking
                        varTypeEnum dstType = max(a->type, b->type);

                        return apply(&sum, a, b, dstType);
                    }
                case '-':       return ex(p->opr.op[0]) - ex(p->opr.op[1]);
                case '*':       return ex(p->opr.op[0]) * ex(p->opr.op[1]);
                case '/':       return ex(p->opr.op[0]) / ex(p->opr.op[1]);
                case '<':       return ex(p->opr.op[0]) < ex(p->opr.op[1]);
                case '>':       return ex(p->opr.op[0]) > ex(p->opr.op[1]);
                case GE:        return ex(p->opr.op[0]) >= ex(p->opr.op[1]);
                case LE:        return ex(p->opr.op[0]) <= ex(p->opr.op[1]);
                case NE:        return ex(p->opr.op[0]) != ex(p->opr.op[1]);
                case EQ:        return ex(p->opr.op[0]) == ex(p->opr.op[1]);


            }
   }
    printf("%s\n", "Unable to evaluate node");
    return 0;
}
