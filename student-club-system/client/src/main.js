import { createApp } from 'vue'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import * as ElementPlusIconsVue from '@element-plus/icons-vue'
import zhCn from 'element-plus/dist/locale/zh-cn.mjs'

// 只注册项目实际使用的图标，减小打包体积
const usedIcons = [
  'School', 'Bell', 'UserFilled', 'User', 'Search', 'OfficeBuilding',
  'Calendar', 'Money', 'Warning', 'WarningFilled', 'Checked', 'Setting',
  'TrendCharts', 'Compass', 'Message', 'Grid', 'Plus', 'Edit', 'Delete',
  'ArrowLeft', 'ArrowRight', 'Close', 'Check', 'InfoFilled', 'CircleCheck',
  'CircleClose', 'Clock', 'List', 'Upload', 'Download', 'Refresh', 'Filter',
  'More', 'Menu', 'Location', 'Star', 'Picture', 'Link', 'Share', 'Lock',
  'Key', 'View', 'Hide', 'ChatDotRound', 'Notebook'
]
const icons = {}
for (const name of usedIcons) {
  if (ElementPlusIconsVue[name]) icons[name] = ElementPlusIconsVue[name]
}

const app = createApp(App)

for (const [key, component] of Object.entries(icons)) {
  app.component(key, component)
}

app.use(ElementPlus, { locale: zhCn })
app.use(router)
app.mount('#app')
