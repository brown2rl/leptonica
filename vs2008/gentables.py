#!/usr/bin/env python

"""
gentables.py -- Converts srcs.csv & progs.cvs to HTML tables that
                sort-filter-table.js can use.

$RCSfile: gentables.py,v $ $Revision: 1.7 $ $Date: 2011/02/16 16:17:21 $
"""

r"""
Requires:

 python 2.6 or greater: activestate.com
  http://activestate.com/Products/Download/Download.plex?id=ActivePython

Uses modified sort-filter-table.js from javascripttoolbox.com.

General Notes:
--------------

Usage:

    python26 gentables.py srcs.csv progs.csv srcDir

    Generates srcs.html, progs.html & functions.html

    To generate ctags file needed by this script, from /src dir do:

        ctags --excmd=number --file-scope=no --c-kinds=-dm --fields=+S *.c

"""

# imports of python standard library modules
# See Python Documentation | Library Reference for details
import csv
import optparse
import os
import re

# ====================================================================

def genTableFromCountDict(countDict, standalone, basename, heading, plural):
    outputFilename = "%s.html" % basename
    print "Writing: %s" % outputFilename
    output = open(outputFilename, "w")

    if standalone:
        output.write('''<html>
<head>
 <link rel="stylesheet" type="text/css" href="sort-filter-table.css"/>
 <script type="text/javascript" src="sort-filter-table-compact.js"></script>
</head>
<body>
''')

    output.write('''
 <table class="centered docutils sorted table-autosort:0
               table-stripeclass:alternate"
               id="%s">
''' % basename)

    keys = countDict.keys()
    keys.sort()
    total = 0

    # Write header row
    output.write('  <thead>\n')
    output.write('   <tr>\n')
    output.write('    <th class="table-sortable:ignorecase" '
                      'title="Click to sort">%s</th>\n' % heading)
    output.write('    <th class="table-sortable:numeric" '
                      'title="Click to sort">#</th>\n')
    output.write('   </tr>\n')
    output.write('  </thead>\n')

    # Generate table body
    output.write('  <tbody>\n')
    for key in keys:
        count = countDict[key]
        total += count
        key = key.replace("&", "&amp;")
        key = key.replace("<", "&lt;")
        key = key.replace(">", "&gt;")

        output.write('   <tr>\n')
        output.write('    <td>%s</td>\n' % key)
        output.write('    <td>%d</td>\n' % count)
        output.write('   </tr>\n')
    output.write('  </tbody>\n')

    # Write table footer
    output.write('  <tfoot>\n')
    output.write('   <tr>\n')
    output.write('    <td>%d %s</td>\n' % (len(keys), plural))
    output.write('    <td>%d</td>\n' % total)
    output.write('   </tr>\n')
    output.write('  </tfoot>\n')

    output.write(' </table>\n')
    if standalone:
        output.write('</body>\n</html>\n')
    output.close()

def genHTMLFromCSV(csvFilename, unsortableColumns, standalone,
                   breakAfterCommasColumn=None, pageLength=None):
    basename, ext = os.path.splitext(csvFilename)
    outputFilename = "%s.html" % basename
    print "Writing: %s" % outputFilename
    output = open(outputFilename, "w")

    if standalone:
        output.write('''<html>
<head>
 <link rel="stylesheet" type="text/css" href="sort-filter-table.css"/>
 <script type="text/javascript" src="sort-filter-table-compact.js"></script>
</head>
<body>
''')

    if pageLength:
        output.write('''
 <table class="centered docutils sorted table-autosort:1 table-autofilter
               table-stripeclass:alternate table-secondary:0
               table-filtered-rowcount:t1filtercount table-rowcount:t1allcount
               table-autopage:%d
               table-page-number:t1page table-page-count:t1pages"
               id="%s">
''' % (pageLength, basename))
    else:
        output.write('''
 <table class="centered docutils sorted table-autosort:1 table-autofilter
               table-stripeclass:alternate table-secondary:0
               table-filtered-rowcount:t1filtercount table-rowcount:t1allcount"
               id="%s">
''' % basename)

    # First count the number of records in the csv file
    nRows = 0
    f = open(csvFilename, "rb")
    reader = csv.reader(f, dialect="excel")
    for row in reader:
        nRows += 1
    f.close()
    nRows += -1

    f = open(csvFilename, "rb")
    reader = csv.reader(f, dialect="excel")

    # Generate header row
    output.write('  <thead>\n')
    output.write('   <tr>\n')
    headings = reader.next()
    for i, column in enumerate(headings):
        if i >= len(headings)-unsortableColumns:
            output.write('    <th>%s</th>\n' % column)
        else:
            output.write('    <th class="table-sortable:ignorecase '
                                        'title="Click to sort">%s</th>\n' % column)
    output.write('   </tr>\n')

    # Generate filter row
    output.write('   <tr>\n')
    for i, column in enumerate(headings):
        if column.lower() == "filename":
            output.write('    <th title="Enter regular expression to match">'
                             '<input name="filter" size="10" '
                              'onkeyup="Table.filter(this,this)">'
                             '</input></th>')
            continue

        if column.lower() == "description":
            output.write('    <th title="Enter regular expression to match">'
                             '<input name="filter" size="30" '
                              'onkeyup="Table.filter(this,this)">'
                             '</input></th>')
            continue

        if column.lower() == "function":
            output.write('    <th title="Enter regular expression to match">'
                             '<input name="filter" size="30" '
                              'onkeyup="Table.filter(this,this)">'
                             '</input></th>')
            continue

        if i >= len(headings)-unsortableColumns:
            output.write('    <th></th>\n')
            continue

        output.write('    <th class="table-filterable"></th>\n')
    output.write('   </tr>\n')

    output.write('''
   <tr>
    <th colspan="%s"><span id="t1filtercount">%d</span>&nbsp;of <span id="t1allcount">%d</span>&nbsp;rows match filter(s)</th>
   </tr>
''' % (len(headings), nRows, nRows))

    if pageLength:
        output.write('''
   <tr>
    <th class="table-page:previous" style="cursor:pointer;"><a href="#">&lt;&lt;Previous</a></th>
    <th colspan="%s">Page <span id="t1page"></span>&nbsp;of <span id="t1pages"></span></th>
    <th class="table-page:next" style="cursor:pointer;"><a href="#">Next&gt;&gt;</a></th>
   </tr>
''' % (len(headings)- 2,))

    output.write('  </thead>\n')

    # Generate table body
    output.write('  <tbody>\n')
    for row in reader:
        output.write('   <tr>\n')
        for i, column in enumerate(row):
            column = column.replace("&", "&amp;")
            column = column.replace("<", "&lt;")
            column = column.replace(">", "&gt;")
            if i == breakAfterCommasColumn:
                column = column.replace(",", ",<br />")

            output.write('    <td>%s</td>\n' % column)
        output.write('   </tr>\n')
    output.write('  </tbody>\n')

    output.write(' </table>\n')
    if standalone:
        output.write('</body>\n</html>\n')
    f.close()
    output.close()

def genSrcSummaryTable(csvFilename, standalone):
    # Count categories
    categoryDict = {}
    f = open(csvFilename, "rb")
    reader = csv.reader(f, dialect="excel")
    reader.next()	#skip header row
    for row in reader:
        filename, category, description = row
        filename = filename.lower()
        root, ext = os.path.splitext(filename)
        count = categoryDict.setdefault(category, 0)
        count += 1
        categoryDict[category] = count
    f.close()

    basename, ext = os.path.splitext(csvFilename)
    basename = "%s-summary" % basename
    genTableFromCountDict(categoryDict, standalone, basename,
                          "Category", "Categories")

def genProgSummaryTables(csvFilename, standalone):
    # Count categories & types
    categoryDict = {}
    typeDict = {}
    f = open(csvFilename, "rb")
    reader = csv.reader(f, dialect="excel")
    reader.next()	#skip header row
    for row in reader:
        filename, type, category, description, examplefile, needsinput = row
        filename = filename.lower()
        root, ext = os.path.splitext(filename)

        count = typeDict.setdefault(type, 0)
        count += 1
        typeDict[type] = count

        count = categoryDict.setdefault(category, 0)
        count += 1
        categoryDict[category] = count
    f.close()

    basename, ext = os.path.splitext(csvFilename)
    typename = "%s-typesummary" % basename
    categoryname = "%s-categorysummary" % basename
    genTableFromCountDict(typeDict, standalone, typename,
                          "Type", "Types")
    genTableFromCountDict(categoryDict, standalone, categoryname,
                          "Category", "Categories")

protoRE = re.compile(
    r"(?ix)"
    r"^LEPT_DLL\s+extern\s+"
    r"(?P<retType> .*? ) \s+"
    r"(?P<funcname> \S+ ) \s+"
    r"\( \s* (?P<signature> [^)]*? ) \s* \)"
    r"\s* ; \s* $"
    )

def genFuncDict(protosFilename, tagsFilename, srcsFilename):
    # map filename to category
    categoryDict = {}
    with open(srcsFilename, "rb") as f:
        reader = csv.reader(f, dialect="excel")
        reader.next()	#skip header row
        for row in reader:
            filename, category, description = row
            filename = filename.lower()
            categoryDict[filename] = category

    funcCategoryDict = {}
    with open(tagsFilename, "r") as f:
        for line in f:
            line = line.strip()
            if len(line) == 0:
                continue
            fields = line.split('\t')
            if fields[0].startswith('!'):
                continue
            if fields[3] != 'f':
                continue
            if len(fields) == 4:
                funcname, filename, lineno, tag = fields
                signature = "()"
            else:
                funcname, filename, lineno, tag, signature = fields

            category = categoryDict[filename]
            if category.lower().endswith("stub functions"):
                continue

            funcCategoryDict[funcname] = (filename, category)

    funcDict = {}
    categoryFuncCountsDict = {}
    with open(protosFilename, "r") as f:
        for line in f:
            line = line.strip()
            if len(line) == 0:
                continue
            if not line.startswith("LEPT_DLL extern"):
                continue
            m = protoRE.search(line)
            if not m:
                continue
            funcname = m.group("funcname")
            category = funcCategoryDict[funcname][1]
            funcDict[funcname] = (funcCategoryDict[funcname][0],
                                  category,
                                  m.group("retType"),
                                  m.group("signature"),
                                  )

            count = categoryFuncCountsDict.setdefault(category, 0)
            count += 1
            categoryFuncCountsDict[category] = count

    return funcDict, categoryFuncCountsDict

def genFuncCSV(funcDict, funcCSVFilename):
    with open(funcCSVFilename, "wb") as f:
        writer = csv.writer(f, dialect="excel")
        writer.writerow( ("Filename", "Category",
                          "Function", "Return Type", "Arguments") )
        functions = funcDict.keys()
        functions.sort()
        for function in functions:
            writer.writerow( (
                funcDict[function][0],
                funcDict[function][1],
                function,
                funcDict[function][2],
                funcDict[function][3],
                ))

def gentables (srcsFilename, progsFilename, protosFilename, tagsFilename,
               standalone):
    funcDict, categoryFuncCountsDict = genFuncDict(protosFilename,
                                                   tagsFilename,
                                                   srcsFilename)
    genFuncCSV(funcDict, "functions.csv")

    genHTMLFromCSV(srcsFilename, 1, standalone)
    genHTMLFromCSV(progsFilename, 3, standalone)
    genHTMLFromCSV("functions.csv", 2, standalone, 4, 250)

    genSrcSummaryTable(srcsFilename, standalone)
    genProgSummaryTables(progsFilename, standalone)

    genTableFromCountDict(categoryFuncCountsDict, standalone,
                          "functions-summary",
                          "Category", "Categories")

# ====================================================================

def main ():
    parser = optparse.OptionParser(usage="%prog [options] srcs.csv progs.csv srcDir",
                                   version="%prog $Revision: 1.7 $")

    parser.set_defaults(debugging=0, standalone=False)
    parser.add_option("-s", "--standalone", action="store_const", const=True,
                      dest="standalone",
                      help="generate standalone HTML files", )
    parser.add_option("-d", "--debug", action="store_const", const=1,
                      dest="debugging",
                      help="basic debugging messages", )
    parser.add_option("-D", "--Debug", action="store_const", const=2,
                      dest="debugging",
                      help="Extended debugging messages", )
    options, args = parser.parse_args()

    if len(args) != 3:
        parser.error("incorrect number of arguments")

    srcsFilename = args[0]
    if not os.path.exists(srcsFilename):
        parser.error("%s doesn't exist" % srcsFilename)

    progsFilename = args[1]
    if not os.path.exists(progsFilename):
        parser.error("%s doesn't exist" % progsFilename)

    srcDir = args[2]
    if not os.path.exists(srcDir):
        parser.error("%s doesn't exist" % srcDir)

    protosFilename = os.path.join(srcDir, "leptprotos.h")
    if not os.path.exists(protosFilename):
        parser.error("%s doesn't exist" % protosFilename)

    tagsFilename = os.path.join(srcDir, "tags")
    if not os.path.exists(tagsFilename):
        parser.error("%s doesn't exist" % tagsFilename)

    gentables(srcsFilename, progsFilename, protosFilename, tagsFilename,
              options.standalone)

if __name__ == '__main__' :
    main()
