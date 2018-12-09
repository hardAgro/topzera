<template>
  <div>
    <div class="mapouter">
      <div class="gmap_canvas">
        <gmap-map
          :center="{lat:10, lng:10}"
          :zoom="3"
          map-type-id="satellite"
          style="width: 100%; height: 100%"
        >
        </gmap-map>
      </div>
    </div>
  </div>
</template>

<script>
import { mapState } from 'vuex'

export default {
  props: {
    sensorId: {
      type: String,
      default: ''
    }
  },

  data () {
    return {
      zoom: 18
    }
  },

  computed: {
  }
}
</script>

<style>
.mapouter {
  text-align:right;
  height:100%;
  width:100%;
  position: absolute;
}
.gmap_canvas {
  overflow:hidden;
  background:none!important;
  height:100%;
  width:100%;
}
</style>
