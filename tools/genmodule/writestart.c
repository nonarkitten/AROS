/*
    Copyright � 1995-2005, The AROS Development Team. All rights reserved.
    $Id$
    
    Function to write modulename_start.c. Part of genmodule.
*/
#include "genmodule.h"

void writestart(struct config *cfg, struct functions *functions)
{
    FILE *out;
    char line[256];
    struct functionhead *funclistit;
    struct functionarg *arglistit;
    struct stringlist *linelistit;
    unsigned int lvo;
    int i;
    
    snprintf(line, 255, "%s/%s_start.c", cfg->gendir, cfg->modulename);
    out = fopen(line, "w");
    if (out==NULL)
    {
	fprintf(stderr, "Could not write %s\n", line);
	exit(20);
    }
    fprintf(out,
	    "/*\n"
	    "    *** Automatically generated file. Do not edit ***\n"
	    "    Copyright � 1995-2004, The AROS Development Team. All rights reserved.\n"
	    "*/\n"
    );
    if (cfg->modtype == DEVICE)
    {
	fprintf(out,
		"#include <exec/io.h>\n"
		"#include <exec/errors.h>\n"
	);
    }
    fprintf(out,
	    "#include <exec/types.h>\n"
	    "#include <exec/libraries.h>\n"
	    "#include <exec/resident.h>\n"
	    "#include <aros/libcall.h>\n"
	    "#include <aros/asmcall.h>\n"
	    "#include <aros/symbolsets.h>\n"
	    "#include <dos/dos.h>\n"
	    "#include <proto/exec.h>\n"
	    "\n"
	    "#include \"%s_libdefs.h\"\n"
	    "\n",
	    cfg->modulename
    );
    if (!(cfg->options & OPTION_NORESIDENT))
    {
	/* Print the library magic and init code. Partly based on code in CLib37x.lha
	 * from Andreas R. Kleinert
	 */
	fprintf(out,
		"#include <exec/initializers.h>\n"
		"#define INITPTR(offset,ptr) 0x80,(UBYTE)offset,(ULONG)ptr\n"
		"\n"
		"extern const int GM_UNIQUENAME(End);\n"
		"extern const APTR GM_UNIQUENAME(FuncTable)[];\n"
		"static const struct InitTable GM_UNIQUENAME(InitTable);\n"
		"static const struct DataTable GM_UNIQUENAME(DataTable);\n"
		"\n"
		"extern const char GM_UNIQUENAME(LibName)[];\n"
		"extern const char GM_UNIQUENAME(LibID)[];\n"
		"extern const char GM_UNIQUENAME(Copyright)[];\n"
		"\n"
	);

	fprintf(out,
		"#define __freebase(lh)\\\n"
		"do {\\\n"
		"    UWORD negsize, possize;\\\n"
		"    UBYTE *negptr = (UBYTE *)lh;\\\n"
		"    negsize = ((struct Library *)lh)->lib_NegSize;\\\n"
		"    possize = ((struct Library *)lh)->lib_PosSize;\\\n"
		"    FreeMem (negptr, negsize+possize);\\\n"
		"} while(0)\n"
		"\n"
	);
	
	fprintf(out,
		"AROS_UFP3 (LIBBASETYPEPTR, GM_UNIQUENAME(InitLib),\n"
		"    AROS_UFPA(LIBBASETYPEPTR, lh, D0),\n"
		"    AROS_UFPA(BPTR, segList, A0),\n"
		"    AROS_UFPA(struct ExecBase *, sysBase, A6)\n"
		");\n"
	);
	if (cfg->modtype != RESOURCE)
	{
	    fprintf(out,
		    "AROS_LP1(BPTR, GM_UNIQUENAME(ExpungeLib),\n"
		    "    AROS_LPA(LIBBASETYPEPTR, lh, D0),\n"
		    "    struct ExecBase *, sysBase, 3, %s\n"
		    ");\n"
		    "\n",
		    cfg->basename
	    );
	}
	fprintf(out,
		"struct Resident const GM_UNIQUENAME(ROMTag) =\n"
		"{\n"
		"    RTC_MATCHWORD,\n"
		"    (struct Resident *)&GM_UNIQUENAME(ROMTag),\n"
		"    (APTR)&GM_UNIQUENAME(End),\n"
		"    RESIDENTFLAGS,\n"
		"    VERSION_NUMBER,\n"
	);
	switch (cfg->modtype)
	{
	case LIBRARY:
	case MUI:
	case MCC:
	case MCP:
	    fprintf(out, "    NT_LIBRARY,\n");
	    break;
	case DEVICE:
	    fprintf(out, "    NT_DEVICE,\n");
	    break;
	case RESOURCE:
	    fprintf(out, "    NT_RESOURCE,\n");
	    break;
	default:
	    fprintf(stderr, "Internal error: unsupported modtype for NT_...\n");
	}
	fprintf(out,
		"    RESIDENTPRI,\n"
		"    (char *)&GM_UNIQUENAME(LibName)[0],\n"
		"    (char *)&GM_UNIQUENAME(LibID)[6],\n"
		"    (APTR)&GM_UNIQUENAME(InitTable)\n"
		"};\n"
		"\n"
		"static struct InitTable\n"
		"{\n"
		"    ULONG                   Size;\n"
		"    const APTR             *FuncTable;\n"
		"    const struct DataTable *DataTable;\n"
		"    APTR                    InitLibTable;\n"
		"}\n"
		"const GM_UNIQUENAME(InitTable) =\n"
		"{\n"
		"    sizeof(LIBBASETYPE),\n"
		"    &GM_UNIQUENAME(FuncTable)[0],\n"
		"    &GM_UNIQUENAME(DataTable),\n"
		"    (APTR)GM_UNIQUENAME(InitLib)\n"
		"};\n"
		"\n"
		"static struct DataTable\n"
		"{\n"
		"    UWORD ln_Type_Init; UWORD ln_Type_Offset; UWORD ln_Type_Content;\n"
		"    UBYTE ln_Name_Init; UBYTE ln_Name_Offset; ULONG ln_Name_Content;\n"
		"    UWORD lib_Flags_Init; UWORD lib_Flags_Offset; UWORD lib_Flags_Content;\n"
		"    UWORD lib_Version_Init; UWORD lib_Version_Offset; UWORD lib_Version_Content;\n"
		"    UWORD lib_Revision_Init; UWORD lib_Revision_Offset; UWORD lib_Revision_Content;\n"
		"    UBYTE lib_IdString_Init; UBYTE lib_IdString_Offset; ULONG lib_IdString_Content;\n"
		"    ULONG ENDMARK;\n"
		"}\n"
		"const GM_UNIQUENAME(DataTable) =\n"
		"{\n"
	);
	switch (cfg->modtype)
	{
	case LIBRARY:
	case MUI:
	case MCC:
	case MCP:
	    fprintf(out, "    INITBYTE(OFFSET(Node, ln_Type), NT_LIBRARY),\n");
	    break;
	case DEVICE:
	    fprintf(out, "    INITBYTE(OFFSET(Node, ln_Type), NT_DEVICE),\n");
	    break;
	case RESOURCE:
	    fprintf(out, "    INITBYTE(OFFSET(Node, ln_Type), NT_RESOURCE),\n");
	    break;
	default:
	    fprintf(stderr, "Internal error: unsupported modtype for NT_...\n");
	}
	fprintf(out,
		"    INITPTR(OFFSET(Node, ln_Name), (ULONG) &GM_UNIQUENAME(LibName)[0]),\n"
	);
	if (cfg->modtype != RESOURCE)
	{
	    fprintf(out,
		    "    INITBYTE(OFFSET(Library, lib_Flags), LIBF_SUMUSED|LIBF_CHANGED),\n"
		    "    INITWORD(OFFSET(Library, lib_Version), VERSION_NUMBER),\n"
		    "    INITWORD(OFFSET(Library, lib_Revision), REVISION_NUMBER),\n"
		    "    INITPTR(OFFSET(Library, lib_IdString), (ULONG) &GM_UNIQUENAME(LibID)[0]),\n"
	    );
	}
	fprintf(out,
		"    (ULONG) 0\n"
		"};\n"
		"\n"
		"const char GM_UNIQUENAME(LibName)[] = NAME_STRING;\n"
		"const char GM_UNIQUENAME(LibID)[] = VERSION_STRING;\n"
		"const char GM_UNIQUENAME(Copyright)[] = COPYRIGHT_STRING;\n"
		"\n"
		"THIS_PROGRAM_HANDLES_SYMBOLSETS\n"
		"DECLARESET(INIT)\n"
		"DECLARESET(EXIT)\n"
		"DECLARESET(CTORS)\n"
		"DECLARESET(DTORS)\n"
		"DECLARESET(INITLIB)\n"
		"DECLARESET(EXPUNGELIB)\n"
		"DECLARESET(OPENLIB)\n"
		"DECLARESET(CLOSELIB)\n"
		"DECLARESET(OPENDEV)\n"
		"DECLARESET(CLOSEDEV)\n"
		"\n"
		"#ifdef SysBase\n"
		"#undef SysBase\n"
		"#endif\n"
		"#define SysBase (GM_SYSBASE_FIELD(lh))\n"
		"\n"
		"AROS_UFH3 (LIBBASETYPEPTR, GM_UNIQUENAME(InitLib),\n"
		"    AROS_UFHA(LIBBASETYPEPTR, lh, D0),\n"
		"    AROS_UFHA(BPTR, segList, A0),\n"
		"    AROS_UFHA(struct ExecBase *, sysBase, A6)\n"
		")\n"
		"{\n"
		"    AROS_USERFUNC_INIT\n"
		"\n"
		"    int ok;\n"
		"\n"
		"    GM_SYSBASE_FIELD(lh) = sysBase;\n"
	);
	if (!(cfg->options & OPTION_NOEXPUNGE) && cfg->modtype!=RESOURCE)
	    fprintf(out, "    GM_SEGLIST_FIELD(lh) = segList;\n");
	if (cfg->options & OPTION_DUPBASE)
	    fprintf(out, "    GM_ROOTBASE_FIELD(lh) = (LIBBASETYPEPTR)lh;\n");
	fprintf(out, "    if ( ");
	if (!(cfg->options & OPTION_NOAUTOLIB))
	    fprintf(out, "set_open_libraries() && ");
	fprintf(out,
		"set_call_funcs(SETNAME(INIT), 1, 1) )\n"
		"    {\n"
		"        set_call_funcs(SETNAME(CTORS), -1, 0);\n"
		"\n"
		"        ok = set_call_libfuncs(SETNAME(INITLIB),1,lh);\n"
		"    }\n"
		"    else\n"
		"        ok = 0;\n"
		"\n"
		"    if (!ok)\n"
		"    {\n"
		"        set_call_libfuncs(SETNAME(EXPUNGELIB),-1,lh);\n"
		"        set_call_funcs(SETNAME(DTORS), 1, 0);\n"
		"        set_call_funcs(SETNAME(EXIT), -1, 0);\n"
	);
	if (!(cfg->options & OPTION_NOAUTOLIB))
	    fprintf(out, "        set_close_libraries();\n");
	fprintf(out,
		"\n"
		"        __freebase(lh);\n"
		"        return NULL;\n"
		"    }\n"
		"    else\n"
		"        return  lh;\n"
		"\n"
		"    AROS_USERFUNC_EXIT\n"
		"}\n"
		"\n"
	);

	switch (cfg->modtype)
	{
	case RESOURCE:
	    break;
	case DEVICE:
	    fprintf(out,
		    "AROS_LH3 (void, GM_UNIQUENAME(OpenLib),\n"
		    "    AROS_LHA(struct IORequest *, ioreq, A1),\n"
		    "    AROS_LHA(ULONG, unitnum, D0),\n"
		    "    AROS_LHA(ULONG, flags, D1),\n"
		    "    LIBBASETYPEPTR, lh, 1, %s\n"
		    ")\n",
		    cfg->basename
	    );
	    fprintf(out,
		    "{\n"
		    "    AROS_LIBFUNC_INIT\n"
		    "\n"
		    "    if ( set_call_libfuncs(SETNAME(OPENLIB), 1, lh)\n"
		    "         && set_call_devfuncs(SETNAME(OPENDEV), 1, ioreq, unitnum, flags, lh)\n"
		    "    )\n"
		    "    {\n"
		    "        ((struct Library *)lh)->lib_OpenCnt++;\n"
		    "        ((struct Library *)lh)->lib_Flags &= ~LIBF_DELEXP;\n"
		    "\n"
		    "        ioreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;\n"
		    "    }\n"
		    "    else\n"
		    "    {\n"
		    "        if (ioreq->io_Error >= 0)\n"
		    "            ioreq->io_Error = IOERR_OPENFAIL;\n"
		    "    }\n"
		    "\n"
		    "    return;\n"
		    "\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n"
	    );
	    break;
	default:
	    fprintf(out,
		    "AROS_LH1 (LIBBASETYPEPTR, GM_UNIQUENAME(OpenLib),\n"
		    "    AROS_LHA (ULONG, version, D0),\n"
		    "    LIBBASETYPEPTR, lh, 1, %s\n"
		    ")\n",
		    cfg->basename
	    );
	    fprintf(out,
		    "{\n"
		    "    AROS_LIBFUNC_INIT\n"
		    "\n"
	    );
	    fprintf(out,
		    "    if ( set_call_libfuncs(SETNAME(OPENLIB), 1, lh) )\n"
		    "    {\n"
	    );
	    if (!(cfg->options & OPTION_DUPBASE))
	    {
		fprintf(out,
			"        ((struct Library *)lh)->lib_OpenCnt++;\n"
			"        ((struct Library *)lh)->lib_Flags &= ~LIBF_DELEXP;\n"
	        );
	    }
	    else
	    {
		fprintf(out,
			"        struct Library *newlib;\n"
			"        UWORD possize = ((struct Library *)lh)->lib_PosSize;\n"
			"\n"
			"        ((struct Library *)lh)->lib_OpenCnt++;\n"
			"        ((struct Library *)lh)->lib_Flags &= ~LIBF_DELEXP;\n"
			"\n"
			"        newlib = MakeLibrary(GM_UNIQUENAME(InitTable).FuncTable,\n"
			"                             GM_UNIQUENAME(InitTable).DataTable,\n"
			"                             NULL,\n"
			"                             GM_UNIQUENAME(InitTable).Size,\n"
			"                             (BPTR)NULL\n"
			"        );\n"
			"        if (newlib == NULL)\n"
			"            return 0;\n"
			"\n"
			"        CopyMem(lh, newlib, possize);\n"
			"        lh = (LIBBASETYPEPTR)newlib;\n"
		);
	    }
	    fprintf(out,
		    "\n"
		    "        return lh;\n"
		    "    }\n"
		    "\n"
		    "    return NULL;\n"
		    "\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n"
	     );
	}

	if (cfg->modtype != RESOURCE)
	{
	    if (cfg->modtype != DEVICE)
		fprintf(out,
			"AROS_LH0 (BPTR, GM_UNIQUENAME(CloseLib),\n"
			"    LIBBASETYPEPTR, lh, 2, %s\n"
			")\n",
			cfg->basename
		);
	    else
		fprintf(out,
			"AROS_LH1(BPTR, GM_UNIQUENAME(CloseLib),\n"
			"    AROS_LHA(struct IORequest *, ioreq, A1),\n"
			"    LIBBASETYPEPTR, lh, 2, %s\n"
			")\n",
			cfg->basename
		);
	
	    fprintf(out,
		    "{\n"
		    "    AROS_LIBFUNC_INIT\n"
		    "\n"
	    );
	    if (!(cfg->options & OPTION_DUPBASE))
	    {
		fprintf(out,
			"    ((struct Library *)lh)->lib_OpenCnt--;\n"
			"    set_call_libfuncs(SETNAME(CLOSELIB),-1,lh);\n"
		);
	    }
	    else
	    {
		fprintf(out,
			"    if (lh != GM_ROOTBASE_FIELD(lh))\n"
			"    {\n"
			"        LIBBASETYPEPTR rootbase = GM_ROOTBASE_FIELD(lh);\n"
			"        __freebase(lh);\n"
			"        set_call_libfuncs(SETNAME(CLOSELIB),-1,lh);\n"
			"        lh = rootbase;\n"
			"    }\n"
			"    ((struct Library *)lh)->lib_OpenCnt--;\n"
			"\n"
		);
	    }
	    if (cfg->modtype == DEVICE)
		fprintf(out,
			"    set_call_devfuncs(SETNAME(CLOSEDEV),-1,ioreq,0,0,lh);\n"
		);
	    if (!(cfg->options & OPTION_NOEXPUNGE))
		fprintf(out,
			"    if\n"
			"    (\n"
			"        (((struct Library *)lh)->lib_OpenCnt == 0)\n"
			"        && (((struct Library *)lh)->lib_Flags & LIBF_DELEXP)\n"
			"    )\n"
			"    {\n"
			"        return AROS_LC1(BPTR, GM_UNIQUENAME(ExpungeLib),\n"
			"                   AROS_LCA(LIBBASETYPEPTR, lh, D0),\n"
			"                   struct ExecBase *, SysBase, 3, %s\n"
			"        );\n"
			"    }\n",
			cfg->basename
		);
	    fprintf(out,
		    "\n"
		    "    return NULL;\n"
		    "\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n"
	    );
	
	    fprintf(out,
		    "AROS_LH1 (BPTR, GM_UNIQUENAME(ExpungeLib),\n"
		    "    AROS_LHA(LIBBASETYPEPTR, lh, D0),\n"
		    "    struct ExecBase *, sysBase, 3, %s\n"
		    ")\n",
		    cfg->basename
	    );
	    fprintf(out,
		    "{\n"
		    "    AROS_LIBFUNC_INIT\n"
		    "\n"
	    );
	    if (!(cfg->options & OPTION_NOEXPUNGE))
	    {
		fprintf(out,
			"\n"
			"    if ( ((struct Library *)lh)->lib_OpenCnt == 0 )\n"
			"    {\n"
			"        BPTR seglist = GM_SEGLIST_FIELD(lh);\n"
			"\n"
			"        Remove((struct Node *)lh);\n"
			"\n"
			"        set_call_libfuncs(SETNAME(EXPUNGELIB),-1,lh);\n"
			"        set_call_funcs(SETNAME(DTORS), 1, 0);\n"
			"        set_call_funcs(SETNAME(EXIT),-1,0);\n"
		);
		if (!(cfg->options & OPTION_NOAUTOLIB))
		    fprintf(out, "        set_close_libraries();\n");
		fprintf(out,
			"\n"
			"        __freebase(lh);\n"
			"\n"
			"        return seglist;\n"
			"    }\n"
			"\n"
			"    ((struct Library *)lh)->lib_Flags |= LIBF_DELEXP;\n"
		);
	    }
	    fprintf(out,
		    "\n"
		    "    return NULL;\n"
		    "\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n"
	    );
	
	    fprintf(out,
		    "AROS_LH0 (LIBBASETYPEPTR, GM_UNIQUENAME(ExtFuncLib),\n"
		    "    LIBBASETYPEPTR, lh, 4, %s\n"
		    ")\n"
		    "{\n"
		    "    AROS_LIBFUNC_INIT\n"
		    "    return NULL;\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n",
		    cfg->basename
	    );
	}
	
	fprintf(out,
		"DEFINESET(INIT)\n"
		"DEFINESET(EXIT)\n"
		"DEFINESET(CTORS)\n"
		"DEFINESET(DTORS)\n"
		"DEFINESET(INITLIB)\n"
		"DEFINESET(EXPUNGELIB)\n"
		"DEFINESET(OPENLIB)\n"
		"DEFINESET(CLOSELIB)\n"
		"DEFINESET(OPENDEV)\n"
		"DEFINESET(CLOSEDEV)\n"
		"\n"
	);
    }

    if (cfg->libcall == REGISTER)
    {
	for (linelistit = cfg->cdeflines; linelistit!=NULL; linelistit = linelistit->next)
	    fprintf(out, "%s\n", linelistit->s);
    }
    
    for (funclistit = functions->funclist; funclistit != NULL; funclistit = funclistit->next)
    {
	switch (funclistit->libcall)
	{
	case STACK:
	    fprintf(out, "int %s();\n", funclistit->name);
	    break;
	    
	case REGISTER:
	    fprintf(out, "%s %s(", funclistit->type, funclistit->name);
	    for (arglistit = funclistit->arguments;
		 arglistit!=NULL;
		 arglistit = arglistit->next
	    )
	    {
		if (arglistit!=funclistit->arguments)
		    fprintf(out, ", ");
		fprintf(out, "%s %s", arglistit->type, arglistit->name);
	    }
	    fprintf(out,
		    ");\nAROS_LH%d(%s, %s,\n",
		    funclistit->argcount, funclistit->type, funclistit->name
	    );
	    for (arglistit = funclistit->arguments;
		 arglistit!=NULL;
		 arglistit = arglistit->next
	    )
	    {
		fprintf(out,
			"         AROS_LHA(%s, %s, %s),\n",
			arglistit->type, arglistit->name, arglistit->reg
		);
	    }
	    fprintf(out,
		    "         %s, %s, %u, %s)\n"
		    "{\n"
		    "    AROS_LIBFUNC_INIT\n\n"
		    "    return %s(",
		    cfg->libbasetypeptrextern, cfg->libbase, funclistit->lvo, cfg->basename,
		    funclistit->name
	    );
	    for (arglistit = funclistit->arguments;
		 arglistit!=NULL;
		 arglistit = arglistit->next
	    )
	    {
		if (arglistit!=funclistit->arguments)
		    fprintf(out, ", ");
		fprintf(out, "%s", arglistit->name);
	    }
	    fprintf(out,
		    ");\n\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n\n");
	    break;
	    
	case REGISTERMACRO:
	    fprintf(out, "int GM_UNIQUENAME(%s)();\n", funclistit->name);
	    break;

	default:
	    fprintf(stderr, "Internal error: unhandled libcall in writestart\n");
	    exit(20);
	    break;
	}
    }

    if (!(cfg->options & OPTION_NORESIDENT))
    {
	fprintf(out,
		"\n"
		"const APTR GM_UNIQUENAME(FuncTable)[]=\n"
		"{\n"
	);
	if (cfg->modtype != RESOURCE)
	{
	    fprintf(out,
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(OpenLib),%s),\n"
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(CloseLib),%s),\n"
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(ExpungeLib),%s),\n"
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(ExtFuncLib),%s),\n",
		    cfg->basename, cfg->basename, cfg->basename, cfg->basename
	    );
	}
	funclistit = functions->funclist;
    }
    else /* NORESIDENT */
    {
	if (cfg->modtype != RESOURCE)
	{
	    int neednull = 0;
	    struct functionhead *funclistit2;
	
	    funclistit = functions->funclist;
	    if (funclistit->lvo != 1)
	    {
		fprintf(stderr, "Module without a generated resident structure has to provide the Open function (LVO==1)\n");
		exit(20);
	    }
	    else
		funclistit = funclistit->next;
	
	    if (funclistit->lvo != 2)
	    {
		fprintf(stderr, "Module without a generated resident structure has to provide the Close function (LVO==2)\n");
		exit(20);
	    }
	    else
		funclistit = funclistit->next;
	
	    if (funclistit->lvo == 3)
		funclistit = funclistit->next;
	    else
		neednull = 1;
	
	    if (funclistit->lvo == 4)
		funclistit = funclistit->next;
	    else
		neednull = 1;

	    if (neednull)
		fprintf(out,
			"\n"
			"AROS_UFH1(static int, %s_null,\n"
			"          AROS_UFHA(struct Library *, libbase, A6)\n"
			")\n"
			"{\n"
			"    AROS_USERFUNC_INIT\n"
			"    return 0;\n"
			"    AROS_USERFUNC_EXIT\n"
			"}\n",
			cfg->modulename
		);
	
	    funclistit = functions->funclist;
	    funclistit2 = funclistit->next;
	    fprintf(out,
		    "\n"
		    "const APTR GM_UNIQUENAME(FuncTable)[]=\n"
		    "{\n"
		    "    &AROS_SLIB_ENTRY(%s,%s),\n"
		    "    &AROS_SLIB_ENTRY(%s,%s),\n",
		    funclistit->name, cfg->basename,
		    funclistit2->name, cfg->basename
	    );
	    funclistit = funclistit2->next;

	    if (funclistit->lvo == 3)
	    {
		fprintf(out, "    &AROS_SLIB_ENTRY(%s,%s),\n",
			funclistit->name, cfg->basename
		);
		funclistit = funclistit->next;
	    }
	    else
		fprintf(out, "    &%s_null,\n", cfg->modulename);
	
	    if (funclistit->lvo == 4)
	    {
		fprintf(out, "    &AROS_SLIB_ENTRY(%s,%s),\n",
			funclistit->name, cfg->basename
		);
		funclistit = funclistit->next;
	    }
	    else
		fprintf(out, "    &%s_null,\n", cfg->modulename);
	}
    }

    lvo = (cfg->modtype == RESOURCE) ? 1 : 4;
    while (funclistit != NULL)
    {
	for (i = lvo+1; i<funclistit->lvo; i++)
	    fprintf(out, "    NULL,\n");
	lvo = funclistit->lvo;
	
	switch (funclistit->libcall)
	{
	case STACK:
	    fprintf(out, "    &%s,\n", funclistit->name);
	    break;
	    
	case REGISTER:
	case REGISTERMACRO:
	    fprintf(out, "    &AROS_SLIB_ENTRY(%s,%s),\n", funclistit->name, cfg->basename);
	    break;
	    
	default:
	    fprintf(stderr, "Internal error: unhandled libcall type in writestart\n");
	    exit(20);
	    break;
	}
	
	funclistit = funclistit->next;
    }

    fprintf(out, "    (void *)-1\n};\n");
    fclose(out);
}
