import arg from 'arg'
import inquirer from 'inquirer'
import fs from 'fs'
import path from 'path'
import { parse } from './ubdl-ts/parser'
import cpp_transpile from './cpp-transpiler/transpiler'
import redux_ts_transpile from './redux-ts-transpiler/transpiler'

function parse_args(args)
{
  const opts = arg({
      '--lang':   String,
      '--outdir': String,
      '--source': String,
      '-l': '--lang',
      '-o': '--outdir',
      '-s': '--source'
    },
    {
      argv: args.slice(2)
    }
  )
  return {
    lang:   opts['--lang']   || 'cpp',
    outdir: opts['--outdir'] || false,
    source: opts['--source'] || null
  }
}

async function promt_for_missing_opts(opts)
{
  const questions = []

  if (!['cpp', 'redux-ts'].includes(opts.lang)) {
    throw `unknown language: ${opts.lang}`
  }

  if (!opts.source) {
    questions.push({
      type: 'input',
      name: 'source',
      message: 'Please select UBDL file:'
    })
  }

  if (!opts.outdir) {
    opts.outdir = path.dirname(opts.source)
  }

  const ans = await inquirer.prompt(questions)
  return {
    ...opts,
    source: opts.source || ans.source
  }
}

function generate_cpp(opts, ubdl)
{
  const cpp_source = cpp_transpile(ubdl)
  const outname = path.basename(opts.source, '.ubdl')
  const outpath = path.join(opts.outdir, `${outname}.h`)
  fs.writeFileSync(outpath, cpp_source)
  console.log(`${outpath} written!`)
}

function generate_redux_ts_store(opts, module, store)
{
  const redux_ts_source = redux_ts_transpile(module, store)
  const outpath = path.join(opts.outdir, `${store.name.toLowerCase()}.ts`)
  fs.writeFileSync(outpath, redux_ts_source)
  console.log(`${outpath} written!`)
}

function generate_redux_ts(opts, ubdl)
{
  ubdl.declarations
    .filter(x => x.kind === 'store')
    .forEach(generate_redux_ts_store.bind(
      null, opts, ubdl.module.join('.')))
}

export async function cli(args)
{
  let opts = parse_args(args)
  opts = await promt_for_missing_opts(opts)
  const source = fs.readFileSync(opts.source).toString()
  try {
    const ubdl = await parse(source)
    switch (opts.lang) {
    case 'cpp':
      generate_cpp(opts, ubdl)
      break
    case 'redux-ts':
      generate_redux_ts(opts, ubdl)
      break
    }
  } catch (e) {
    console.log(e)
  }
}
