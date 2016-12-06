#include "ast.h"
#include "symboles.h"

// Cette variable globale vaut au depart 0, et est incrementee a chaque fois
int countDigraph;

// Cette variable permet de declarer des etiquettes a la volee
int currentLabel;

nodeType *createNumericNode(float v)
{
	nodeType *p;

	if ((p=(nodeType*)malloc(sizeof(nodeType))) == NULL)
	{
		printf("out of memory error\n");
		exit(1);
	}

	p->type=typeNumeric;
	p->t_numeric.valeur=v;
	
	return p;
}

nodeType *createOperatorNode(int oper, int nops, ...) 
{
    	va_list ap;
    	nodeType *p;
    	int i;

    	/* allocate node */
    	if ((p = (nodeType*)malloc(sizeof(nodeType))) == NULL)
		{
			printf("out of memory error\n");
			exit(1);
		}
    	if ((p->t_oper.op = (nodeType**)malloc(nops * sizeof(nodeType))) == NULL)
		{
			printf("out of memory error\n");
			exit(1);
		}

    	/* copy information */
    	p->type = typeOperator;
    	p->t_oper.oper = oper;
    	p->t_oper.nOperands = nops;
    	va_start(ap, nops);
    	for (i = 0; i < nops; i++)
        	p->t_oper.op[i] = va_arg(ap, nodeType*);
    	va_end(ap);
    	return p;
}

nodeType *createIdentifierNode(char *id, int funcNum, int index)
{
        nodeType *p;

        if ((p=malloc(sizeof(nodeType))) == NULL)
        {
                printf("out of memory error\n");
                exit(1);
        }

        p->type=typeIdentifier;
        p->t_identifier.ident=strdup(id);
        p->t_identifier.funcNum=funcNum;
        p->t_identifier.index=index;

        return p;
}


// TO DO: explain this well in rapport
void generateAsmRec(nodeType *n, FILE *fout)
{
	int label1, label2;

	if (n==NULL)
		return;

	switch (n->type)
	{
		case typeOperator:
			{
				switch (n->t_oper.oper)
				{
					case OPER_ASSIGN:
                    	{
                    	    nodeType *n0=n->t_oper.op[0];
							printf("ident=%s func=%d index=%d\n",n0->t_identifier.ident,n0->t_identifier.funcNum, n0->t_identifier.index);
							// global							
							if (n0->t_identifier.funcNum==0)
							{
                            	fprintf(fout,"\tpush\t%d\n", n0->t_identifier.index);
                               	generateAsmExpression(n->t_oper.op[1],fout);
                               	fprintf(fout,"\tstm\n");
							}
							// local
							else
							{
								// rajouter du code
                            	fprintf(fout,"\tlibp\t%d\n", n0->t_identifier.index);
                               	generateAsmExpression(n->t_oper.op[1],fout);
                               	fprintf(fout,"\tstm\n");
							}
                        }
						break;
					case OPER_SEQUENCE:
						generateAsmRec(n->t_oper.op[0],fout);
						generateAsmRec(n->t_oper.op[1],fout);
						break;
					case OPER_WRITE:
						generateAsmExpression(n->t_oper.op[0],fout);
						fprintf(fout,"\toutput\n");
						break;
					case OPER_SKIP:
						break;
					case OPER_RESERVE_SPACE:
						fprintf(fout,"\tinc\t%d\n",table_nbre_variables_globales[0]);
						fprintf(fout,"\tjp\tmain\n");
						break;
					case OPER_MAIN:
						fprintf(fout,"main:\n");
						generateAsmRec(n->t_oper.op[0],fout);
						break;
					case OPER_RETURN:
						{
							fprintf(fout, "\tret\n");
							// rajouter du code
						}
						break;
					case OPER_DEF_FONCTION:
						{
							fprintf(fout, "%s: \n", table_ident_fonctions[0][getFunctionNum()].ident);
							generateAsmRec(n->t_oper.op[1],fout);
							// rajouter du code
						}
						break;
                    
                    // while (exp)
                    //    bloc
                    //
                    // exp = op[0] ; bloc = op[1]
					case OPER_WHILE:
						fprintf(fout,"L%03d:\n", label1 = currentLabel++); // 03 = 1000
                        generateAsmExpression(n->t_oper.op[0],fout); 
            			fprintf(fout,"\tjf\tL%03d\n", label2 = currentLabel++); // jump si false
						generateAsmRec(n->t_oper.op[1],fout);
        				fprintf(fout,"\tjp\tL%03d\n", label1); // jump to label1
    					fprintf(fout,"L%03d:\n", label2); 
						break;
					
                    // if EXP then
                    //      bloc 1
                    // else
                    //      bloc 2
                    //
                    // EXP = op[0] , bloc1 = op[1] , bloc2 = op[2]
                    case OPER_IF:
                        generateAsmExpression(n->t_oper.op[0],fout);
						if (n->t_oper.nOperands==3)
						{
            				fprintf(fout,"\tjf\tL%03d\n", label1 = currentLabel++); 
							generateAsmRec(n->t_oper.op[1],fout);        
        					fprintf(fout,"\tjp\tL%03d\n", label2 = currentLabel++); // jump to label 2
     						fprintf(fout,"L%03d:\n", label1);
    	   					generateAsmRec(n->t_oper.op[2],fout);      // bloc 2
      						fprintf(fout,"L%03d:\n", label2);
						}
						else
						{
            				fprintf(fout,"\tjf\tL%03d\n", label1 = currentLabel++);
							generateAsmRec(n->t_oper.op[1],fout);
            				fprintf(fout,"L%03d:\n", label1);
						}
						break;
					default:
						break;
				}
			}
			break;
		case typeNumeric:
		case typeIdentifier:
			break;
	}
}

void generateAsmExpression(nodeType *n, FILE *fout)
{
        if (n==NULL)
                return;

        switch (n->type)
        {
                case typeNumeric:
                {
                        fprintf(fout,"\tpushr\t%f\n",n->t_numeric.valeur);
                }
                break;
                case typeIdentifier:
                {
					int funcNum=n->t_identifier.funcNum;
					int index=n->t_identifier.index;
					printf("ident=%s func=%d index=%d\n",n->t_identifier.ident,funcNum,index);
					if (n->t_identifier.funcNum==0)
					{
	                	fprintf(fout,"\tpush\t%d\n", n->t_identifier.index);
	                   	fprintf(fout,"\tmts\n");
					}
					else
					{
						// rajouter du code
	                	fprintf(fout,"\tlibp\t%d\n", n->t_identifier.index);
	                    fprintf(fout,"\tmts\n");
					}
				}
                break;
                case typeOperator:
                        {
                                switch (n->t_oper.oper)
                                {
                                        case OPER_ADD:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\tadd\n");
                                                break;
                                        case OPER_SUB:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\tsub\n");
                                                break;
                                        case OPER_MULT:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\tmult\n");
                                                break;
                                        case OPER_DIV:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\tdiv\n");
                                                break;
                                        case OPER_INF:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\tls\n");
                                                break;
                                        case OPER_SUP:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\tgt\n");
                                                break;
                                        case OPER_EQ:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\teq\n");
                                                break;
                                        case OPER_NE:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                fprintf(fout,"\teq\n");
                                                fprintf(fout,"\tnot\n");
                                                break;
                                        case OPER_SEQUENCE:
                                                generateAsmExpression(n->t_oper.op[0],fout);
                                                generateAsmExpression(n->t_oper.op[1],fout);
                                                break;
                                        case OPER_APPEL_FONCTION:
											{
												// rajouter du code
												fprintf(fout, "\tinc 1\n" ); // reserve space for value return
												generateAsmRec(n->t_oper.op[1],fout);	// empile 
												fprintf(fout, "\tsavebp\n" );

												int f_num = est_deja_declare(0, n->t_oper.op[0]->t_identifier.ident);

												fprintf(fout, "\tinc %d\n", table_nbre_variables_locales[f_num]);
												fprintf(fout, "\tcall %s\n", table_ident_fonctions[0][getFunctionNum()].ident);
												fprintf(fout, "\tdec %d \n", table_nbre_variables_locales[f_num]);

												fprintf(fout, "\trstrbp\n" );
												fprintf(fout, "\tdec %d \n", table_nbre_formels[f_num]);

											}
                                                break;
                                        default:
                                                break;
                                }
                        }
                        break;
        }
}

void generateAsm(nodeType *n, char *filename)
{
	FILE *fout;

	currentLabel=0;
	fout=fopen(filename,"w");
	generateAsmRec(n,fout);
	fprintf(fout,"\thalt\n");
	fprintf(fout,"\tend\n");
	fclose(fout);	
}

void generateDigraphNameNode(nodeType *n,FILE *fout)
{
/*
        if (n==NULL)
                return;

        switch (n->type)
        {
                case typeNumeric:
                        {
				n->digraphNode=countDigraph++;
                                fprintf(fout,"\tA%3.3d [label=\"%f\"]\n",n->digraphNode,n->t_numeric.valeur);
                        }
                        break;
                case typeIdentifier:
                        {
				n->digraphNode=countDigraph++;
                                fprintf(fout,"\tA%3.3d [label=\"%s\"]\n",n->digraphNode,n->t_identifier.ident);
                        }
                        break;
                case typeOperator:
                        {
                                switch (n->t_oper.oper)
                                {
                                        case OPER_APPEL_FONCTION:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"appel fonc\"]\n",n->digraphNode);
                                                break;
                                        case OPER_ADD:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"+\"]\n",n->digraphNode);
                                                break;
                                        case OPER_SUB:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"-\"]\n",n->digraphNode);
                                                break;
                                        case OPER_MULT:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"*\"]\n",n->digraphNode);
                                                break;
                                        case OPER_DIV:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"/\"]\n",n->digraphNode);
                                                break;
                                        case OPER_INF:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"<\"]\n",n->digraphNode);
                                                break;
                                        case OPER_SUP:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\">\"]\n",n->digraphNode);
                                                break;
                                        case OPER_EQ:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"==\"]\n",n->digraphNode);
                                                break;
                                        case OPER_NE:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"!=\"]\n",n->digraphNode);
                                                break;
                                        case OPER_NOT:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                n->digraphNode=countDigraph++;
                                                fprintf(fout,"\tA%3.3d [label=\"not\"]\n",n->digraphNode);
                                                break;
                                        case OPER_SKIP:
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"skip\"]\n",n->digraphNode);
                                                break;
                                        case OPER_RETURN:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"return\"]\n",n->digraphNode);
                                                break;
                                        case OPER_WRITE:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"write\"]\n",n->digraphNode);
                                                break;
                                        case OPER_DEF_FONCTION:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"def_fonction\"]\n",n->digraphNode);
                                                break;
                                        case OPER_ASSIGN:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"=\"]\n",n->digraphNode);
                                                break;
                                        case OPER_WHILE:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"while\"]\n",n->digraphNode);
                                                break;
                                        case OPER_IF:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
						if (n->t_oper.nOperands==3)
                                                	generateDigraphNameNode(n->t_oper.op[2],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"if\"]\n",n->digraphNode);
                                                break;
                                        case OPER_SEQUENCE:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
                                                generateDigraphNameNode(n->t_oper.op[1],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\";\"]\n",n->digraphNode);
                                                break;
                                        case OPER_RESERVE_SPACE:
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"reserve_space\"]\n",n->digraphNode);
                                                break;
                                        case OPER_MAIN:
                                                generateDigraphNameNode(n->t_oper.op[0],fout);
						n->digraphNode=countDigraph++;
                                		fprintf(fout,"\tA%3.3d [label=\"main\"]\n",n->digraphNode);
                                                break;
                                        default:
                                                break;
                                }
                        }
                        break;
        }
*/
}

void generateDigraphEdges(nodeType *n,FILE *fout)
{
        if (n==NULL)
                return;

        switch (n->type)
        {
                case typeNumeric:
                case typeIdentifier:
                        break;
                case typeOperator:
                {
		//printf("oper=%d\n",n->t_oper.oper);
                        switch (n->t_oper.oper)
                        {
                                case OPER_ADD:
                                case OPER_SUB:
                                case OPER_MULT:
                                case OPER_DIV:
                                case OPER_INF:
                                case OPER_SUP:
                                case OPER_EQ:
                                case OPER_NE:
                                case OPER_WHILE:
                                case OPER_APPEL_FONCTION:
                                case OPER_DEF_FONCTION:
                                case OPER_ASSIGN:
                                case OPER_SEQUENCE:
                                case OPER_RETURN:
                                        fprintf(fout,"\tA%3.3d -> A%3.3d\n", n->digraphNode,n->t_oper.op[0]->digraphNode);
                                        fprintf(fout,"\tA%3.3d -> A%3.3d\n", n->digraphNode,n->t_oper.op[1]->digraphNode);
                                        generateDigraphEdges(n->t_oper.op[0],fout);
                                        generateDigraphEdges(n->t_oper.op[1],fout);
                                        break;
                                case OPER_IF:
                                        fprintf(fout,"\tA%3.3d -> A%3.3d\n", n->digraphNode,n->t_oper.op[0]->digraphNode);
                                        fprintf(fout,"\tA%3.3d -> A%3.3d\n", n->digraphNode,n->t_oper.op[1]->digraphNode);
										if (n->t_oper.nOperands==3)
                                        	fprintf(fout,"\tA%3.3d -> A%3.3d\n", n->digraphNode,n->t_oper.op[2]->digraphNode);
                                        generateDigraphEdges(n->t_oper.op[0],fout);
                                        generateDigraphEdges(n->t_oper.op[1],fout);
										if (n->t_oper.nOperands==3)
                                        	generateDigraphEdges(n->t_oper.op[2],fout);
                                        break;
                                case OPER_WRITE:
                                case OPER_NOT:
								case OPER_MAIN:
                                        fprintf(fout,"\tA%3.3d -> A%3.3d\n", n->digraphNode,n->t_oper.op[0]->digraphNode);
                                        generateDigraphEdges(n->t_oper.op[0],fout);
                                        break;
								case OPER_SKIP:
								case OPER_RESERVE_SPACE:
									break;
                                default:
                                    break;
                        }
                }
                        break;
        }
}

void generateDigraph(nodeType *n)
{
/*
	FILE *fout;

	fout=fopen("res.dot","w");
	countDigraph=0;
	fprintf(fout,"digraph {\n");
	printf("generateDigraphNameNode\n");
	generateDigraphNameNode(n,fout);
	printf("generateDigraphEdges\n");
	generateDigraphEdges(n,fout);
	fprintf(fout,"}\n");
	fclose(fout);
	system("dot -Tpng res.dot -o res.png");
*/
}
