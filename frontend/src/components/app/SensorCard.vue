<template>
  <v-card>
    <v-card-title
      class="pb-0"
    >
      <div class="headline">
        {{ name }}
      </div>
    </v-card-title>

    <v-card-text>
      <v-layout
        v-if="flow !== null"
        row
        justify-space-around
        class="pl-3 pr-3"
      >
        <span
          :class="irregular ? 'red--text' : 'grey--text'"
          class="display-1 font-weight-thin mr-auto"
        >
          {{ flow.toFixed(2) }} l/min

          <v-icon
            v-if="irregular"
            color="red"
            large
          >
            {{ icon }}
          </v-icon>
        </span>

        <v-tooltip
          bottom
          color="black"
        >
          <v-icon
            v-if="irregular"
            slot="activator"
            color="yellow"
            x-large
          >
            mdi-alert
          </v-icon>

          <span class="white--text">{{ alertTooltipMessage }}</span>
        </v-tooltip>
      </v-layout>

      <v-layout
        v-if="flow === null"
        row
        justify-space-around
        class="pl-3 pr-3"
      >
        <v-progress-linear indeterminate />
      </v-layout>
    </v-card-text>
    <v-card-actions class="pb-5">
      <span
        v-if="lastTimestamp"
        :class="irregular ? 'red--text' : 'grey--text'"
      >
        Última leitura às {{ lastTimestamp }}
      </span>
      <span v-else />
    </v-card-actions>
  </v-card>
</template>

<script>
import _ from 'lodash'

export default {
  props: {
    id: {
      type: String,
      default: ''
    },

    name: {
      type: String,
      default: ''
    },

    flowMax: {
      type: Number,
      default: 100
    },

    flowMin: {
      type: Number,
      default: 1
    },

    readings: {
      type: Array,
      default: () => []
    }
  },

  computed: {
    irregular () {
      return this.flow >= this.flowMax || this.flow <= this.flowMin
    },

    alertTooltipMessage () {
      if (this.flow >= this.flowMax) return 'Fluxo acima do normal'
      else if (this.flow <= this.flowMin) return 'Fluxo abaixo do normal'
      return ''
    },

    icon () {
      if (this.flow >= this.flowMax) return 'arrow_upward'
      else if (this.flow <= this.flowMin) return 'arrow_downward'
      return ''
    },

    flow () {
      return this.readings.length ? _.last(this.readings).flow : null
    },

    lastTimestamp () {
      return this.readings.length ? _.last(this.readings).timestamp : null
    }
  }
}
</script>
