import fs from 'fs'
import path from 'path'
import mustache, { render } from 'mustache'

mustache.escape = x => x // disable HTML escape

const STORE_TEMPLATE_PATH = path.join(__dirname, 'templates/store.mst')

function camel_to_snake(s)
{
  return s.split(/(?=[A-Z])/).join('_').toLowerCase()
}

function ts_type(type)
{
  switch (type) {
    case 'number': return 'number'
    case 'string': return 'string'
    default: {
      if (type.kind === 'list') {
        return `${ts_type(type.element_type)}[]`
      } else {
        throw 'unexpected'
      }
    }
  }
}

function comma_it(a)
{
  if (a.length <= 1) {
    a = a.map(x => ({ ...x, comma: false }))
  } else {
    a = a.map(x => ({ ...x, comma: true }))
    a[a.length - 1].comma = false
  }
  return a
}

function map_property(p)
{
  return {
    prop_name: p.name,
    prop_type: ts_type(p.type)
  }
}

function map_action(a)
{
  return {
    action_name: a.name,
    ACTION_SNAKE_NAME: camel_to_snake(a.name).toUpperCase(),
    action_type_name: `${a.name[0].toUpperCase()}${a.name.slice(1)}`,
    params: comma_it(a.params.map(x => ({
      param_name: x.name,
      param_type: ts_type(x.type)
    })))
  }
}

export default function(module, s)
{
  const template = fs.readFileSync(STORE_TEMPLATE_PATH).toString()
  return render(template, {
    module,
    store_name: s.name,
    properties: comma_it(s.members.filter(x => x.kind === 'property').map(map_property)),
    actions: comma_it(s.members.filter(x => x.kind === 'action').map(map_action))
  })
}
